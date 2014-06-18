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
fake them using the set wrapper-operators ~use~ on the native single-unit
operators.

The ~use~ operators passes single values to an operator one-by-one value and
collects all returned values/streams of values in a flat stream of values.
This does not work for some binary predicates, like equal, but one could 
implement an ordered pairwise comparison here.

It may be useful, to have some operators consuming a stream of units and
returning an aggregated vale, as e.g. initial, final, present, never, always.

December 2006, Christian D[ue]ntgen: Moved class functions for unit types to 
where they belong (ie. ~TemporalAlgebra~).

----

State Operator/Signatures

OK    makemvalue   (**)  stream (tuple ([x1:t1,xi:uT,..,xn:tn])) -->  mT
OK    the_mvalue   (**)            stream uT --> mT
OK    get_duration                   periods --> duration
OK    point2d                        periods --> point
OK    queryrect2d                    instant --> rect
OK    circle              point x real x int --> region
OK    uint2ureal:                       uint --> ureal

      the_unit:  For T in {bool, int, string, region*}
           *: Crashed for T=region
OK          point  point  instant instant bool bool --> upoint
OK          ipoint ipoint bool    bool              --> upoint
OK          real real real bool instant instant bool bool --> ureal
OK          iT duration bool   bool     --> uT
OK          T instant instant bool bool --> uT

      the_ivalue:  For T in {bool, int, string, real, point, region}
OK                               (instant T) --> iT

OK    isempty                             U  --> bool (for U in kind UNIT)
OK    trajectory                      upoint --> line
OK    velocity                        mpoint --> mpoint
OK    velocity                        upoint --> upoint
OK    derivable                        mreal --> mbool
OK    derivable                        ureal --> ubool
OK    derivative                       mreal --> mreal
OK    derivative                       ureal --> ureal
OK    speed               mpoint [ x geoid ] --> mreal
OK    speed               upoint [ x geoid ] --> ureal

      passes:  For T in {bool, int, string, point}:
OK  +                            uT x      T --> bool
OK  +                         ureal x   real --> bool
n/a +                       uregion x region --> bool

OK  + deftime      (**)                   uT --> periods
OK  + atinstant    (**)         uT x instant --> iT
OK  + atperiods    (**)         uT x periods --> (stream uT)
OK  + Initial      (**)                   uT --> iT
OK    Initial      (**)          (stream uT) --> iT
OK  + final        (**)                   uT --> iT
OK    final        (**)          (stream uT) --> iT
OK  + present      (**)         uT x instant --> bool
OK  + present      (**)         uT x periods --> bool
n/a   present          (stream uT) x instant --> bool (use use2/present)
n/a   present          (stream uT) x periods --> bool (use use2/present)

      atmax:  For T in {bool, int, real, string}:
OK  +                                     uT --> (stream uT)

      atmin:  For T in {bool, int, real, string}:
OK  +                                     uT --> (stream uT)

      at:     For T in {bool, int, string, point, region*}
OK  +       uT x       T --> (stream uT)     as intersection: uT x T
OK       ureal x    real --> (stream ureal)  as intersection: ureal  x real
OK  +   upoint x  region --> (stream upoint) as intersection: upoint x region

      distance:  T in {int, point}
OK  -           uT x    uT --> ureal
OK  ?           uT x     T --> ureal
OK  ?            T x    uT --> ureal
OK  +        ureal x ureal --> (stream ureal)
OK  +        ureal x  real --> (stream ureal)
OK  +         real x ureal --> (stream ureal)

OK  + abs:           ureal --> (stream ureal)
OK  + abs:            uint --> uint

     intersection: For T in {bool, int, string}:
OK  +          uT x      uT --> (stream uT)
OK  +          uT x       T --> (stream uT)     same as at: uT  x T
OK  +           T x      uT --> (stream uT)     same as at: uT  x T
OK  +       ureal x    real --> (stream ureal)  same as at: ureal  x real
OK  +        real x   ureal --> (stream ureal)  same as at: ureal  x real
OK  -      upoint x   point --> (stream upoint) same as at: upoint x point
OK  -       point x  upoint --> (stream upoint) same as at: upoint x point
OK  -      upoint x  upoint --> (stream upoint)
OK  +      upoint x    line --> (stream upoint)
OK  +        line x  upoint --> (stream upoint)
OK  +       ureal x   ureal --> (stream ureal)
OK  +      upoint x uregion --> (stream upoint) same as at: upoint x uregion
OK  -     uregion x  upoint --> (stream upoint)
OK  -      upoint x  region --> (stream upoint) same as: at: upoint x region
OK  -      region x  upoint --> (stream upoint)
n/a +      upoint x  points --> (stream upoint)
n/a +      points x  upoint --> (stream upoint)
Pre -     uregion x  region --> (stream uregion)
Pre -      region x uregion --> (stream uregion)

OK  + no_components:     uT --> uint

OK  + and, or: ubool x ubool --> ubool
OK  +           bool x ubool --> ubool
OK  +          ubool x  bool --> ubool

      ==, ##, <<, >>, <<==, >>==:
      For T in {bool, int, string, real, point, region}
Test+                uT x uT --> bool

=, #, <, >, <=, >=: T in {bool, int, string, real}
OK   +                uT x      uT --> (stream ubool)
OK   +                 T x      uT --> (stream ubool)
OK   +                 T x      uT --> (stream ubool)
OK   +            upoint x  upoint --> (stream ubool) for {=,#} only
OK   +             point x  upoint --> (stream ubool) for {=,#} only
OK   +            upoint x   point --> (stream ubool) for {=,#} only
pre  +           uregion x uregion --> (stream ubool) for {=,#} only
n/a  +            region x uregion --> (stream ubool) for {=,#} only
n/a  +           uregion x  region --> (stream ubool) for {=,#} only


OK  + not:             ubool --> ubool

  inside:
OK  +      upoint x uregion --> (stream ubool)
pre +      upoint x    line --> (stream ubool)
n/a +      upoint x  points --> (stream ubool)
n/a +     uregion x  points --> (stream ubool)

n/a + mdirection:    upoint --> ureal
n/a + mheading:      upoint --> ureal

n/a + area: uregion --> ureal             see TemporalLiftedAlgebra

OK  + sometimes: (       ubool) --> bool
OK               (stream ubool) --> bool
OK  + never:     (       ubool) --> bool
OK               (stream ubool) --> bool
OK  + always:    (       ubool) --> bool
OK               (stream ubool) --> bool

OK    length: (upoint [geoid]) --> real

COMMENTS:

(*):  These operators have been implemented for
      T in {bool, int, real, point}
(**): These operators have been implemented for
      T in {bool, int, real, point, string, region}

Key to STATE of implementation:

   OK : Operator has been implemented and fully tested
  (OK): Operator has been implemented and partially tested
  Test: Operator has been implemented, but tests have not been done
  ERR : Operator produces errors
  Pre : Operator has not been functionally implemented, but
        stubs (dummy code) exist
  n/a : Neither functionally nor dummy code exists for this ones

    + : Equivalent exists for according mType
    - : Does nor exist for according mType
    ? : It is unclear, whether it exists or not

----
*/

/*
August 2006, Christian D[ue]ntgen: Added missing checks for ~undefined~
argument values to all value mapping functions. The functions will now
return ~undefined~ result values for these cases.

Changed structure of the file to become ordered by operators rather than by
typemapping, valuemapping etc. This makes the file easier to extend.

September 2009 Simone Jandt: Changed TU\_VM\_ComparePredicateValue\_UReal 
to use new member function CompUReal of UReal.

*/

/*
0. Bug-List

----

 (none known)

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

#include "TemporalUnitAlgebra.h"
#include "NestedList.h"
#include "NList.h"
#include "QueryProcessor.h"
#include "AlgebraManager.h"
#include "Algebra.h"
#include "StandardTypes.h"
#include "SpatialAlgebra.h"
#include "Geoid.h"
#include "RelationAlgebra.h"
#include "TemporalAlgebra.h"
#include "TemporalExtAlgebra.h"
#include "MovingRegionAlgebra.h"
#include "RectangleAlgebra.h"
#include "PolySolver.h"
#include "ListUtils.h"
#include "Symbols.h"
#include "Stream.h"
#include "GenericTC.h"
#include "RefinementStream.h"

extern NestedList* nl;
extern QueryProcessor* qp;
extern AlgebraManager* am;


#include "DateTime.h"
using namespace datetime;

// #define TUA_DEBUG

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
3 Auxiliary Functions

*/


/*
Auxiliary Method ~TU\_GetMidwayInstant~

Returns the instant at the middle of the interval defined by ~start~ and  ~end~.

*/

Instant TU_GetMidwayInstant(const Instant &start, const Instant &end)
{
  DateTime result(instanttype);
  result = start + ((end - start) / 2);
  return result;
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

Is used for the ~deftime~,~atinstant~,~atperiods~ operations.

*/

int
UnitSimpleSelect( ListExpr args )
{
  ListExpr T = nl->First( args );
  if( nl->SymbolValue( T ) == UBool::BasicType() )
    return 0;
  if( nl->SymbolValue( T ) == UInt::BasicType() )
    return 1;
  if( nl->SymbolValue( T ) == UReal::BasicType() )
    return 2;
  if( nl->SymbolValue( T ) == UPoint::BasicType() )
    return 3;
  if( nl->SymbolValue( T ) == UString::BasicType() )
    return 4;
  if( nl->SymbolValue( T ) == URegion::BasicType() )
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
      if( nl->SymbolValue( arg1 ) == UBool::BasicType() )
        return 0;
      if( nl->SymbolValue( arg1 ) == UInt::BasicType() )
        return 1;
      if( nl->SymbolValue( arg1 ) == UReal::BasicType() )
        return 2;
      if( nl->SymbolValue( arg1 ) == UPoint::BasicType() )
        return 3;
      if( nl->SymbolValue( arg1 ) == UString::BasicType() )
        return 4;
      if( nl->SymbolValue( arg1 ) == URegion::BasicType() )
        return 5;
    }

  if(   !( nl->IsAtom( arg1 ) )
      && ( nl->ListLength(arg1) == 2 )
      && ( TypeOfRelAlgSymbol(nl->First(arg1)) == stream ) )
    { if( nl->IsEqual( nl->Second(arg1), UBool::BasicType() ) )
        return 6;
      if( nl->IsEqual( nl->Second(arg1), UInt::BasicType() ) )
        return 7;
      if( nl->IsEqual( nl->Second(arg1), UReal::BasicType() ) )
        return 8;
      if( nl->IsEqual( nl->Second(arg1), UPoint::BasicType() ) )
        return 9;
      if( nl->IsEqual( nl->Second(arg1), UString::BasicType() ) )
        return 10;
      if( nl->IsEqual( nl->Second(arg1), URegion::BasicType() ) )
        return 11;
    }

  return -1; // This point should never be reached
}


/*
5 Implementation of Algebra Operators

5.1 Operator ~speed~

5.1.1 Type mapping function for ~speed~

Type mapping for ~speed~ is

----
  upoint [ x geoid ] --> ureal
  mpoint [ x geoid ] --> mreal

----

*/
ListExpr
TypeMapSpeed( ListExpr args )
{
  int noargs = nl->ListLength( args );
  string errmsg = "Expected ( {upoint|mpoint} [ x geoid ] ).";
  if( (noargs < 1) || (noargs >2) ) {
    return listutils::typeError(errmsg +" 1");
  }
  if(    (noargs==2)
      && !listutils::isSymbol(nl->Second(args), Geoid::BasicType()) ){
    return listutils::typeError(errmsg +" 2");
  }
  ListExpr first = nl->First(args);
  if( listutils::isSymbol(first, UPoint::BasicType()) ) {
    return nl->SymbolAtom( UReal::BasicType() );
  }
  if( listutils::isSymbol(first, MPoint::BasicType()) ) {
    return nl->SymbolAtom( MReal::BasicType() );
  }
  return listutils::typeError(errmsg +" 3");
}

/*
5.1.2 Value mapping for operator ~speed~

*/

int MPointSpeed(Word* args, Word& result, int message, Word& local, Supplier s)
{
  result = (qp->ResultStorage( s ));
  MReal  *res   = (MReal*)result.addr;
  MPoint *input = (MPoint*)args[0].addr;
  Geoid* g = 0;
  res->Clear();

  if ( !input->IsDefined() ){
    res->SetDefined(false);
    return 0;
  }
  if( qp->GetNoSons(s)==2 ){
    g = static_cast<Geoid*>(args[1].addr);
    if(!g->IsDefined()){
      res->SetDefined(false);
      return 0;
    }
  }
  // call member function:
  input->MSpeed( *res, g );
  return 0;
}

int UnitPointSpeed(Word* args,Word& result,int message,Word& local,Supplier s)
{
  result = qp->ResultStorage( s );
  UReal  *res   = (UReal*)result.addr;
  UPoint *input = (UPoint*)args[0].addr;

  Geoid* g = 0;
  if ( !input->IsDefined() ){
    res->SetDefined(false);
    return 0;
  }
  if( qp->GetNoSons(s)==2 ){ // using (LON,LAT) and Geoid
    g = static_cast<Geoid*>(args[1].addr);
    if(!g->IsDefined()){
      res->SetDefined(false);
      return 0;
    }
  }
  input->USpeed( *res, g );
  return 0;
}

/*
5.1.3 Specification for operator ~speed~

*/

const string
TemporalSpecSpeed  =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>mpoint [x geoid] -> mreal\n"
"upoint [x geoid] -> ureal</text--->"
"<text>speed( Obj [, Geoid] )</text--->"
"<text>Returns the scalar speed of a spatio-temporal object Obj in "
"unit/s. If Geoid is used, coordinates in Obj are expected to be geographic "
"(LON,LAT) coordinates and velocity is calculated in accordance to the geoid. "
"Otherwise metric (X,Y) coordinates are assumed.</text--->"
"<text>query speed(mp1)</text---> ) )";

/*
5.1.4 Selection Function of operator ~speed~

*/
int
SpeedSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );

  if( nl->SymbolValue( arg1 ) == MPoint::BasicType() )
    return 0;

  if( nl->SymbolValue( arg1 ) == UPoint::BasicType() )
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

    if( nl->IsEqual( arg1, Instant::BasicType() )  )
      return nl->SymbolAtom( Rectangle<2>::BasicType() );
  }
  return nl->SymbolAtom( Symbol::TYPEERROR() );
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
"<text>Translate an instant object to a rect object to query against "
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

    if( nl->IsEqual( arg1, Periods::BasicType() )  )
      return nl->SymbolAtom( Point::BasicType() );
  }
  return nl->SymbolAtom( Symbol::TYPEERROR() );
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
          Interval<Instant> intv1, intv2;

          range->Get( 0, intv1 );
          range->Get( range->GetNoComponents()-1, intv2 );
          sup = intv1.end;
          inf = intv2.start;
          Y = sup.ToDouble(); // Derives the maximum of all intervals.
          X = inf.ToDouble(); // Derives the minimum of all intervals.
          ((Point*)result.addr)->Set(X,Y); // Returns the calculated point.
          ((Point*)result.addr)->SetDefined(true);
        }
      else
        { // empty periods -> set to (mininstant, mininstant)
          ((Point*)result.addr)->SetDefined( false );
        }
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
"<text>Translates a periods value to a point value representing "
"the period's total deftime interval. The empty periods value "
"is mapped to the undefined point.</text--->"
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
5.4 Operator ~get\_duration~

5.4.1 Type Mapping for ~get\_duration~

Type mapping for ~get\_duration~ is

----  periods  [->]  duration

----

*/
ListExpr
PeriodsTypeMapGetDuration( ListExpr args )
{
  if ( nl->ListLength( args ) == 1 )
  {
    ListExpr arg1 = nl->First( args );

    if( nl->IsEqual( arg1, Periods::BasicType() )  )
      return nl->SymbolAtom( Duration::BasicType() );
  }
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

/*
5.4.2 Value Mapping for ~get\_duration~

*/
int GetDuration( Word* args, Word& result, int message,
                 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  Periods* range = (Periods*)args[0].addr;
  DateTime* Res = ((DateTime*)result.addr);
  *Res = DateTime(0, 0, durationtype);
  if ( !range->IsDefined() )
    ((DateTime*)result.addr)->SetDefined( false );
  else
    {
      Res->SetDefined(true);
      if( !range->IsEmpty()  )
        {
          Interval<Instant> intv;

          for( int i = 0; i < range->GetNoComponents(); i++ )
            {
              range->Get( i, intv );
              *Res += (intv.end - intv.start);
            }
        }
    }
  return 0;
}

/*
5.4.3 Specification for operator ~get\_duration~

*/
const string
TemporalSpecGetDuration  =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>(periods) -> duration</text--->"
"<text>get_duration( _ )</text--->"
"<text>Return the duration in spanned by a periods value "
"as a duration value.</text--->"
"<text>query get_duration(periods)</text---> ) )";

/*
5.4.4 Selection Function of operator ~get\_duration~

Not necessary.

*/


/*
5.4.5  Definition of operator ~get\_duration~

*/
Operator temporalunitget_duration( "get_duration",
                      TemporalSpecGetDuration,
                      GetDuration,
                      Operator::SimpleSelect,
                      PeriodsTypeMapGetDuration );

/*
5.5 Operators ~makemvalue~, ~the\_mvalue~

This operators create a moving object type mT from a stream of unit type
objects uT. The operator does not expect the stream to be ordered by their
timeintervals. Also, undefined units are allowed (but will be ignored).
If the stream contains amindst 2 units with overlapping timeIntervals,
the operator might crash. If the stream is empty, the result will be an
empty mT.

5.5.1 Type Mapping for ~makemvalue~, ~the\_mvalue~

Type mapping for ~makemvalue~ is

----

      stream (tuple ([x1:t1,x1:ubool,..,[xn:tn)))   ->  mbool
              APPEND (i ti)
      stream (tuple ([x1:t1,x1:uint,..,[xn:tn)))    ->  mint
              APPEND (i ti)
      stream (tuple ([x1:t1,x1:ureal,..,[xn:tn)))   ->  mreal
              APPEND (i ti)
      stream (tuple ([x1:t1,x1:upoint,..,[xn:tn)))  ->  mpoint
              APPEND (i ti)
      stream (tuple ([x1:t1,x1:ustring,..,[xn:tn)))  ->  mstring
              APPEND (i ti)

      NOT YET AVAILABLE DUE TO ERRORS:
      stream (tuple ([x1:t1,x1:uregion,..,[xn:tn)))  ->  mregion
              APPEND (i ti)

----

For ~the\_mvalue~, it is

----
      the_mvalue:  (stream uT) -->  mT
----

*/

ListExpr TU_TM_themvalue( ListExpr args )
{
  string errmsg = "Expected (stream T), where T in{ubool,uint,upoint,ustring}";
  if(nl->ListLength(args) != 1){
    return listutils::typeError(errmsg);
  }
  if(!listutils::isDATAStream(nl->First(args))){
    return listutils::typeError(errmsg);
  }
  ListExpr elemtype = nl->Second(nl->First(args));
  if(listutils::isSymbol(elemtype,UBool::BasicType())){
    return nl->SymbolAtom( MBool::BasicType() );
  }
  if(listutils::isSymbol(elemtype,UInt::BasicType())){
    return nl->SymbolAtom( MInt::BasicType() );
  }
  if(listutils::isSymbol(elemtype,UString::BasicType())){
    return nl->SymbolAtom( MString::BasicType() );
  }
  if(listutils::isSymbol(elemtype,UReal::BasicType())){
    return nl->SymbolAtom( MReal::BasicType() );
  }
  if(listutils::isSymbol(elemtype,UPoint::BasicType())){
    return nl->SymbolAtom( MPoint::BasicType() );
  }
//   if(listutils::isSymbol(elemtype,URegion::BasicType())){
//     return nl->SymbolAtom( MRegion::BasicType() );
//   }
  return listutils::typeError(errmsg);
}

ListExpr the_mvalue2TM(ListExpr args){
  string err = "stream(UT) , T in {bool. int, real, string, point} expected";
  if(!nl->HasLength(args,1)){
    return  listutils::typeError(err);
  }
  ListExpr a = nl->First(args);
  if(Stream<UBool>::checkType(a)) return nl->SymbolAtom(MBool::BasicType());
  if(Stream<UInt>::checkType(a)) return nl->SymbolAtom(MInt::BasicType());
  if(Stream<UString>::checkType(a)) return nl->SymbolAtom(MString::BasicType());
  if(Stream<UReal>::checkType(a)) return nl->SymbolAtom(MReal::BasicType());
  if(Stream<UPoint>::checkType(a)) return nl->SymbolAtom(MPoint::BasicType());
  return  listutils::typeError(err);
}




ListExpr MovingTypeMapMakemvalue( ListExpr args )

{
  ListExpr first, second, rest, listn,
           lastlistn, first2, second2, firstr, listfull, attrtype;

  int j;
  string argstr, argstr2, attrname, inputtype, inputname, fulllist;

  //check the list length.
  if(nl->ListLength(args)!=2){
    return listutils::typeError("two arguments expected");
  }

  first = nl->First(args);
  nl->WriteToString(argstr, first);

  // check the structure of the list.
  if( !listutils::isTupleStream(first) )
  {
    ErrorReporter::ReportError("Operator makemvalue expects as first argument "
      "a tuplestream, but gets '" + argstr + "'.");
    return nl->TypeError();
  }

  // check the given parameter
  second  = nl->Second(args);
  nl->WriteToString(argstr, second);
  if(argstr == Symbol::TYPEERROR()){
    return listutils::typeError("invalid attrname" + argstr);
  }

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


  if(inputtype==""){
    return listutils::typeError("attribute not found");
  }

  if((inputtype != UBool::BasicType()) &&
     (inputtype != UInt::BasicType()) &&
     (inputtype != UReal::BasicType()) &&
     (inputtype != UPoint::BasicType()) &&
     (inputtype != UString::BasicType()) ) {
    return listutils::typeError("attr type not in {ubool, uint,"
                                " ustring, ureal, upoint");
  }
  attrname = nl->SymbolValue(second);
  j = FindAttribute(nl->Second(nl->Second(first)), attrname, attrtype);

  assert( j !=0 );

  if( inputtype == UPoint::BasicType() )
    attrtype = nl->SymbolAtom( MPoint::BasicType() ) ;
  if( inputtype == UBool::BasicType() )
    attrtype = nl->SymbolAtom( MBool::BasicType() );
  if( inputtype == UReal::BasicType() )
    attrtype = nl->SymbolAtom( MReal::BasicType() );
  if( inputtype == UInt::BasicType() )
    attrtype = nl->SymbolAtom( MInt::BasicType() );
  if( inputtype == UString::BasicType() )
    attrtype = nl->SymbolAtom( MString::BasicType() );
//if( inputtype == URegion::BasicType() )
//  attrtype = nl->SymbolAtom( MRegion::BasicType());

  return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
           nl->TwoElemList(nl->IntAtom(j),
           nl->StringAtom(nl->SymbolValue(attrtype))), attrtype);

  // Appending the number of the attribute in the relation is very important,
  // because in the other case we can't work with it in the value function.
}

/*
5.5.2 Value Mapping for ~makemvalue~, ~the\_mvalue~

*/

template <class Mapping, class Unit>
int MappingMakemvalue(Word* args,Word& result,int message,
                      Word& local,Supplier s)
{
  Mapping* m;
  Unit* unit;
  Word currentTupleWord;

  assert(args[2].addr != 0); // assert existence of input
  assert(args[3].addr != 0); // assert existence of input

  int attributeIndex = ((CcInt*)args[2].addr)->GetIntval() - 1;

  qp->Open(args[0].addr);
  qp->Request(args[0].addr, currentTupleWord);

  result = qp->ResultStorage(s);

  m = (Mapping*) result.addr;
  m->Clear();
  m->SetDefined( true );
  m->StartBulkLoad();

  while ( qp->Received(args[0].addr) ) // get all tuples
    {
      Tuple* currentTuple = (Tuple*)currentTupleWord.addr;
      Attribute* currentAttr = (Attribute*)currentTuple->
                                              GetAttribute(attributeIndex);
      if(currentAttr == 0)
      {
        cout << endl << "ERROR in " << __PRETTY_FUNCTION__
             << ": received Nullpointer!" << endl;
        assert( false );
      }
      else if(currentAttr->IsDefined())
      {
        unit = static_cast<Unit*>(currentAttr);
        m->Add( *unit );
      } else {
        cerr << endl << __PRETTY_FUNCTION__ << ": Dropping undef unit. "
             << endl;
      }
      currentTuple->DeleteIfAllowed();
      qp->Request(args[0].addr, currentTupleWord);
    }
  m->EndBulkLoad( true, true ); // force Mapping to sort the units
  qp->Close(args[0].addr);      // and mark invalid Mapping as undefined

  return 0;
}

template <class Unit>
int UCompare( const void *a, const void *b )
{
  Unit *unita = new ((void*)a) Unit,
       *unitb = new ((void*)b) Unit;

  int cmp= unita->Compare(unitb);
  return cmp;
}

template <class Mapping, class Unit>
int MappingMakemvaluePlain(Word* args,Word& result,int message,
                      Word& local,Supplier s)
{
  Word currentUnit;
  Unit* unit1;
  Instant lastInst(instanttype, 0);
  bool lastRC= false;
  DbArray<Unit> allUnits(0);
  qp->Open(args[0].addr);
  qp->Request(args[0].addr, currentUnit);

  result = qp->ResultStorage(s);
  Mapping* m = static_cast<Mapping*>(result.addr);
  m->Clear();
  m->SetDefined( true );

  while ( qp->Received(args[0].addr) ) { // get all tuples
      unit1 = static_cast<Unit*>(currentUnit.addr);
      if(unit1 == 0) {
        cout << endl << __PRETTY_FUNCTION__ << ": Received Nullpointer!"
             << endl;
        assert( false );
      } else if(unit1->IsDefined()) {
          allUnits.Append(*unit1);
      } else {
        cerr << endl << __PRETTY_FUNCTION__ << ": Dropping undef unit "
             << endl;
      }
      unit1->DeleteIfAllowed();
      qp->Request(args[0].addr, currentUnit);
  }
  qp->Close(args[0].addr);
  if(allUnits.Size() == 0) return 0;

  allUnits.Sort( UCompare<Unit> );

  
  Unit unit2;
  allUnits.Get(0, unit2);
  lastInst= unit2.timeInterval.start;
  for(int i=0; i< allUnits.Size(); ++i)
  {
    allUnits.Get(i, unit2);
    if(unit2.timeInterval.start > lastInst ||
        ((unit2.timeInterval.start == lastInst) &&
          !(unit2.timeInterval.lc && lastRC) ))
    {
      m->MergeAdd( unit2 );
      lastInst= unit2.timeInterval.end;
      lastRC= unit2.timeInterval.rc;
    }
  }

  return 0;
}


template<class Mapping, class Unit>
int the_mvalue2VM1(Word* args, Word& result, int message,
                 Word& local, Supplier s){

  result = qp->ResultStorage(s);
  Mapping* res = (Mapping*)result.addr;
  res->Clear();
  res->StartBulkLoad();
  res->SetDefined(true);
  Stream<Unit> stream(args[0]);
  Unit* unit;
  stream.open();
  unit = stream.request();
  Unit* lastUnit = 0;
  while(unit){
    if(unit->IsDefined()){
      if(lastUnit==0){ // first Unit
         res->MergeAdd(*unit);
         lastUnit = unit;
      } else { // not the first unit
         if((unit->timeInterval.start > lastUnit->timeInterval.end) ||
            ( (unit->timeInterval.start == lastUnit->timeInterval.end) &&
              (!unit->timeInterval.lc || !lastUnit->timeInterval.rc))){
             // case : unit starts after lastUnit
            res->MergeAdd(*unit);
            lastUnit->DeleteIfAllowed();
            lastUnit = unit;
         } else if((unit->timeInterval.end < lastUnit->timeInterval.end) ||
                   ((unit->timeInterval.end ==lastUnit->timeInterval.end) && 
                    (!unit->timeInterval.rc || lastUnit->timeInterval.rc))) {
            // no part of the unit if after the last unit
            // ignore this unit
            unit->DeleteIfAllowed();
         } else {
            // unit overlaps lastUnit, but is longer than lastUnit
           Interval<Instant> iv = unit->timeInterval;
           iv.start = lastUnit->timeInterval.end;
           iv.lc = !lastUnit->timeInterval.rc;
           Unit nUnit(true);
           unit->AtInterval(iv,nUnit);
           res->MergeAdd(nUnit);
           lastUnit->DeleteIfAllowed();
           lastUnit = unit;;  
         }
      }
    } // unit is defined
    unit = stream.request();
  }
  if(lastUnit){
     lastUnit->DeleteIfAllowed();
  }
  stream.close();

  res->EndBulkLoad(false); 
  return 0;
}





// here comes the version for mregion, where URegion has a rather
// ugly implementation and thus needs a specialized treatment!
int MappingMakemvalue_movingregion(Word* args,Word& result,int message,
                              Word& local,Supplier s)
{
  MRegion* m;
  Word currentTupleWord;

  assert(args[2].addr != 0); // assert existence of input
  assert(args[3].addr != 0); // assert existence of input

  int attributeIndex = ((CcInt*)args[2].addr)->GetIntval() - 1;

  qp->Open(args[0].addr);
  qp->Request(args[0].addr, currentTupleWord);

  result = qp->ResultStorage(s);

  m = (MRegion*) result.addr;
  m->Clear();
  m->SetDefined( true );
  m->StartBulkLoad();

  while ( qp->Received(args[0].addr) ) // get all tuples
    {
      Tuple* currentTuple = (Tuple*)currentTupleWord.addr;
      URegion* currentUnit = static_cast<URegion*>(currentTuple->
        GetAttribute(attributeIndex));

      if(currentUnit == 0) {
        cout << endl << __PRETTY_FUNCTION__ << ": Received Nullpointer!"<< endl;
        assert( false );
      } else if(currentUnit->IsDefined()) {
          m->AddURegion( *currentUnit );
      } else {
        cerr << endl << __PRETTY_FUNCTION__ << ": Dropping undef URegion "
             << endl;
      }
      currentTuple->DeleteIfAllowed();
      qp->Request(args[0].addr, currentTupleWord);
    }
  m->EndBulkLoad( true, true );  // force Mapping to sort the units
  qp->Close(args[0].addr);       // and mark invalid Mapping as undefined

  return 0;
}

int MappingMakemvalue_movingregionPlain(Word* args,Word& result,int message,
                              Word& local,Supplier s)
{
  MRegion* m;
  URegion* unit;
  Word currentUnit;

  qp->Open(args[0].addr);
  qp->Request(args[0].addr, currentUnit);

  result = qp->ResultStorage(s);

  m = (MRegion*) result.addr;
  m->Clear();
  m->SetDefined( true );
  m->StartBulkLoad();

  while ( qp->Received(args[0].addr) ) // get all tuples
    {
      unit = (URegion*) currentUnit.addr;
      if(unit == 0) {
        cout << endl << __PRETTY_FUNCTION__ << ":received Nullpointer!" << endl;
        assert( false );
      } else if(unit->IsDefined()) {
          m->AddURegion( *unit );
      } else {
        cerr << endl << __PRETTY_FUNCTION__ << ": Dropping undef URegion "
            << endl;
      }
      unit->DeleteIfAllowed();
      qp->Request(args[0].addr, currentUnit);
    }
  m->EndBulkLoad( true, true ); // force Mapping to sort the units
  qp->Close(args[0].addr);      // and mark invalid Mapping as undefined

  return 0;
}

/*
5.5.3 Specification for operators ~makemvalue~, ~the\_mvalue~

*/
const string
TemporalSpecMakemvalue  =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>For T in {bool, int, string, real, point, region*}:\n"
"((stream (tuple ((x1 t1)...(xn tn))) (uT)))-> mT\n"
"*: Not yet available</text--->"
"<text>_ makemvalue[ _ ]</text--->"
"<text>Create a moving object from a (not necessarily sorted) "
"tuple stream containing a unit type attribute. "
"No two unit timeintervals may overlap. Undefined units are "
"allowed and will be ignored. A stream with less than 1 defined "
"unit will result in an 'empty' moving object, not in an 'undef'.</text--->"
"<text>query units(zug5) transformstream makemvalue[elem]</text---> ) )";

const string
TemporalSpecThemvalue  =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>For T in {bool, int, string, real, point, region*}:\n"
"(stream uT) -> mT\n"
"*: Not yet available</text--->"
"<text>_ the_mvalue</text--->"
"<text>Create a moving object from a (not necessarily sorted) "
"object stream containing units. If two unit time intervals overlap, the unit "
"that starts first is kept in the result, and the unit that starts later is "
"ignored. Undefined units are allowed and will be ignored. A stream with less "
"than 1 defined unit will result in an 'empty' moving object, not in an "
"'undef'.</text--->"
"<text>query units(zug5) the_mvalue</text---> ) )";


const string
the_mvalue2Spec  =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>For T in {bool, int, string, real, point}:\n"
"(stream uT) -> mT\n"
"</text--->"
"<text>_ the_mvalue2</text--->"
"<text>Create a moving object from a sorted "
"object stream containing units. "
"Two units may overlap in time. The second unit will be shortened. "
"</text--->"
"<text>query units(zug5) the_mvalue2</text---> ) )";


/*
5.5.4 Selection Function of operators ~makemvalue~, ~the\_mvalue~

*/
int
ThemvalueSelect( ListExpr args )
{
  string argstr;
  nl->WriteToString(argstr, args);
  if ( argstr == "((stream ubool))" )   return 0;
  if ( argstr == "((stream uint))" )    return 1;
  if ( argstr == "((stream ustring))" ) return 2;
  if ( argstr == "((stream ureal))" )   return 3;
  if ( argstr == "((stream upoint))" )  return 4;
  if ( argstr == "((stream uregion))" ) return 5;
  return -1; // This point should never be reached
}

int
the_mvalue2Select( ListExpr args )
{
  ListExpr t = nl->Second(nl->First(args));
  if(UBool::checkType(t)) return 0;
  if(UInt::checkType(t)) return 1;
  if(UString::checkType(t)) return 2;
  if(UReal::checkType(t)) return 3;
  if(UPoint::checkType(t)) return 4;
  return -1;
}


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
  if( inputtype == UBool::BasicType() )   return 0;
  if( inputtype == UInt::BasicType() )    return 1;
  if( inputtype == UString::BasicType() ) return 2;
  if( inputtype == UReal::BasicType() )   return 3;
  if( inputtype == UPoint::BasicType() )  return 4;
  if( inputtype == URegion::BasicType() ) return 5;

  return -1; // This point should never be reached
}

ValueMapping temporalmakemvaluemap[] = {
      MappingMakemvalue<MBool, UBool>,
      MappingMakemvalue<MInt, UInt>,
      MappingMakemvalue<MString, UString>,
      MappingMakemvalue<MReal, UReal>,
      MappingMakemvalue<MPoint, UPoint>,
      MappingMakemvalue_movingregion} ;

ValueMapping temporalthemvaluemap[] = {
      MappingMakemvaluePlain<MBool, UBool>,
      MappingMakemvaluePlain<MInt, UInt>,
      MappingMakemvaluePlain<MString, UString>,
      MappingMakemvaluePlain<MReal, UReal>,
      MappingMakemvaluePlain<MPoint, UPoint>,
      MappingMakemvalue_movingregionPlain };


ValueMapping the_mvalue2VM[] = {
      the_mvalue2VM1<MBool, UBool>,
      the_mvalue2VM1<MInt, UInt>,
      the_mvalue2VM1<MString, UString>,
      the_mvalue2VM1<MReal, UReal>,
      the_mvalue2VM1<MPoint, UPoint> };

/*
5.5.5  Definition of operators ~makemvalue~, ~the\-mvalue~

*/
Operator temporalunitmakemvalue( "makemvalue",
                        TemporalSpecMakemvalue,
                        6,
                        temporalmakemvaluemap,
                        MakemvalueSelect,
                        MovingTypeMapMakemvalue );

Operator temporalunitthemvalue( "the_mvalue",
                        TemporalSpecThemvalue,
                        6,
                        temporalthemvaluemap,
                        ThemvalueSelect,
                        TU_TM_themvalue );


Operator the_mvalue2( "the_mvalue2",
                      the_mvalue2Spec,
                      5,
                      the_mvalue2VM,
                      the_mvalue2Select,
                      the_mvalue2TM);

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

    if( nl->IsEqual( arg1, UPoint::BasicType() ) )
      return nl->SymbolAtom( Line::BasicType() );
  }
  return nl->SymbolAtom( Symbol::TYPEERROR() );
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
  if ( upoint->IsDefined() ){
    line->SetDefined( true );
    upoint->UTrajectory( *line );   // call memberfunction
  } else {
    line->SetDefined( false );
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
"unit point object. Static upoint objects "
"yield empty line objects, on undef argument, it returns undef.</text--->"
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

   if(  nl->IsEqual( arg1, UBool::BasicType() )  ||
        nl->IsEqual( arg1, UInt::BasicType() )   ||
        nl->IsEqual( arg1, UReal::BasicType() )  ||
        nl->IsEqual( arg1, UPoint::BasicType() ) ||
        nl->IsEqual( arg1, UString::BasicType() ) ||
        nl->IsEqual( arg1, URegion::BasicType() ) )
   return nl->SymbolAtom( Periods::BasicType() );
  }
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

/*
5.7.2 Value Mapping for ~deftime~

*/
template <class Unit>
int MappingUnitDefTime( Word* args, Word& result, int message,
                        Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  Periods* r = ((Periods*) result.addr);
  Unit*    m = ((Unit*)    args[0].addr);

  r->Clear();
  if ( !m->IsDefined() )
    r->SetDefined( false );
  else
    {
      r->SetDefined( true );
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

    if( nl->IsEqual( arg2, Instant::BasicType() ) )
    {
       if( nl->IsEqual( arg1, UBool::BasicType() ) )
        return nl->SymbolAtom( IBool::BasicType() );

      if( nl->IsEqual( arg1, UInt::BasicType() ) )
        return nl->SymbolAtom( IInt::BasicType() );

      if( nl->IsEqual( arg1, UReal::BasicType() ) )
        return nl->SymbolAtom( IReal::BasicType() );

      if( nl->IsEqual( arg1, UPoint::BasicType() ) )
        return nl->SymbolAtom( IPoint::BasicType() );

      if( nl->IsEqual( arg1, UString::BasicType() ) )
        return nl->SymbolAtom( IString::BasicType() );

      if( nl->IsEqual( arg1, URegion::BasicType() ) )
        return nl->SymbolAtom( IRegion::BasicType() );
    }
  }
  return nl->SymbolAtom( Symbol::TYPEERROR() );
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

  if ( !t->IsDefined() || !posUnit->IsDefined() ) {
    pResult->SetDefined( false );
  } else if( posUnit->timeInterval.Contains(*t) ) {
      posUnit->TemporalFunction( *t, pResult->value );
      pResult->instant = *t;
      pResult->SetDefined( true );
  } else {    // instant not contained by deftime interval
    pResult->SetDefined( false );
  }
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
      return nl->SymbolAtom( Symbol::TYPEERROR() );
    }

  arg1 = nl->First( args );
  arg2 = nl->Second( args );

  nl->WriteToString(argstr, arg2);
  if ( !( nl->IsEqual( arg2, Periods::BasicType() ) ) )
    {
      ErrorReporter::ReportError("Operator atperiods expects a second argument"
                             " of type 'periods' but gets '" + argstr + "'.");
      return nl->SymbolAtom( Symbol::TYPEERROR() );
    }

  if( nl->IsAtom( arg1 ) )
    {
      if( nl->IsEqual( arg1, UBool::BasicType() ) )
        return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
               nl->SymbolAtom(UBool::BasicType()));
      if( nl->IsEqual( arg1, UInt::BasicType() ) )
        return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
               nl->SymbolAtom(UInt::BasicType()));
      if( nl->IsEqual( arg1, UReal::BasicType() ) )
        return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
               nl->SymbolAtom(UReal::BasicType()));
      if( nl->IsEqual( arg1, UPoint::BasicType() ) )
        return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                               nl->SymbolAtom(UPoint::BasicType()));
      if( nl->IsEqual( arg1, UString::BasicType() ) )
        return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
               nl->SymbolAtom(UString::BasicType()));
      if( nl->IsEqual( arg1, URegion::BasicType() ) )
        return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
               nl->SymbolAtom(URegion::BasicType()));

      nl->WriteToString(argstr, arg1);
      ErrorReporter::ReportError("Operator atperiods expects a first argument "
                                 "of type T in {ubool, uint, ureal, upoint, "
                                 "ustring, uregion} but gets a '"
                                 + argstr + "'.");
      return nl->SymbolAtom( Symbol::TYPEERROR() );
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
          return nl->SymbolAtom( Symbol::TYPEERROR() );
        }

      if( nl->IsEqual( nl->Second(arg1), UBool::BasicType() ) )
        return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                               nl->SymbolAtom(UBool::BasicType()));
      if( nl->IsEqual( nl->Second(arg1), UInt::BasicType() ) )
        return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                               nl->SymbolAtom(UInt::BasicType()));
      if( nl->IsEqual( nl->Second(arg1), UReal::BasicType() ) )
         return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                                nl->SymbolAtom(UReal::BasicType()));
      if( nl->IsEqual( nl->Second(arg1), UPoint::BasicType() ) )
         return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                                 nl->SymbolAtom(UPoint::BasicType()));
      if( nl->IsEqual( nl->Second(arg1), UString::BasicType() ) )
         return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                                 nl->SymbolAtom(UString::BasicType()));
      if( nl->IsEqual( nl->Second(arg1), URegion::BasicType() ) )
         return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                                 nl->SymbolAtom(URegion::BasicType()));

      nl->WriteToString(argstr, nl->Second(arg1));
      ErrorReporter::ReportError("Operator atperiods expects a type "
                              "(stream T); T in {ubool, uint, ureal, upoint, "
                              "ustring, uregion} but gets '(stream "
                              + argstr + ")'.");
      return nl->SymbolAtom( Symbol::TYPEERROR() );
    };

  nl->WriteToString( argstr, args );
  ErrorReporter::ReportError("Operator atperiods encountered an "
                             "unmatched typerror for arguments '"
                             + argstr + "'.");
  return nl->SymbolAtom( Symbol::TYPEERROR() );
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
  Interval<Instant> interval;
  Alpha *unit;
  Alpha r(true);
  Periods* periods;

  switch( message )
  {
  case OPEN:

// #ifdef TUA_DEBUG
//     cout << "\nMappingUnitAtPeriods: OPEN" << endl;
// #endif

    localinfo = new AtPeriodsLocalInfo;
    localinfo->uWord = args[0];
    localinfo->pWord = args[1];
    localinfo->j = 0;
    local.setAddr(localinfo);
    return 0;

  case REQUEST:

// #ifdef TUA_DEBUG
//     cout << "\nMappingUnitAtPeriods: REQUEST" << endl;
// #endif
    if( local.addr == 0 )
      return CANCEL;
    localinfo = (AtPeriodsLocalInfo *)local.addr;
    unit      =   (Alpha*)localinfo->uWord.addr;
    periods   = (Periods*)localinfo->pWord.addr;

    if( !unit->IsDefined()    ||
        !periods->IsDefined() ||
        periods->IsEmpty()       )
    {
      result.setAddr(0);
      return CANCEL;
    }
// #ifdef TUA_DEBUG
//     cout << "   Unit's timeInterval u="
//          << TUPrintTimeInterval( unit->timeInterval ) << endl;
// #endif
    if( localinfo->j >= periods->GetNoComponents() )
      {
        result.setAddr(0);
// #ifdef TUA_DEBUG
//           cout << "Maquery train7 inside train7sectionsppingUnitAtPeriods: "
//                << "REQUEST finished: CANCEL (1)"
//                << endl;
// #endif
        return CANCEL;
      }
    periods->Get( localinfo->j, interval );
    localinfo->j++;
// #ifdef TUA_DEBUG
//     cout << "   Probing timeInterval p ="
//          << TUPrintTimeInterval(interval)
//          << endl;
// #endif
    while( interval.Before( unit->timeInterval ) &&
           localinfo->j < periods->GetNoComponents() )
    { // forward to first candidate interval
        periods->Get(localinfo->j, interval);
        localinfo->j++;
// #ifdef TUA_DEBUG
//         cout << "   Probing timeInterval="
//             << TUPrintTimeInterval(interval)
//             << endl;
//         if (interval.Before( unit->timeInterval ))
//           cout << "     p is before u" << endl;
//         if (localinfo->j < periods->GetNoComponents())
//           cout << "   j < #Intervals" << endl;
// #endif
    }

    if( unit->timeInterval.Before( interval ) )
      { // interval after unit-deftime --> finished
        result.addr = 0;
// #ifdef TUA_DEBUG
//           cout << "MappingUnitAtPeriods: REQUEST finished: CANCEL (2)"
//                << endl;
// #endif
        return CANCEL;
    }

    if(unit->timeInterval.Intersects( interval ))
    { // interval intersectd unit's deftime --> produce result
        // create unit restricted to interval
        unit->AtInterval( interval, r );
        Alpha* aux = new Alpha( r );
        result.setAddr( aux );
// #ifdef TUA_DEBUG
//             cout << "   Result interval="
//                  << TUPrintTimeInterval(aux->timeInterval)
//                  << endl;
//             cout << "   Result defined=" << aux->IsDefined()
//                  << endl;
//             cout << "MappingUnitAtPeriods: REQUEST finished: YIELD"
//                  << endl;
// #endif
        return YIELD;
    }

    if( localinfo->j >= periods->GetNoComponents() )
    { // Passed last interval --> finished
      result.addr = 0;
// #ifdef TUA_DEBUG
//       cout << "MappingUnitAtPeriods: REQUEST finished: CANCEL (3)"
//         << endl;
// #endif
      return CANCEL;
    }

    result.setAddr(0 );
    cout << "MappingUnitAtPeriods: REQUEST finished: CANCEL (4)"
         << endl;
    cout << "Intervals should overlap: " << endl;
    cout << "  Unit's timeInterval = ";
    TUPrintTimeInterval(unit->timeInterval);
    cout << "  Current Period's interval = ";
    TUPrintTimeInterval(interval);
    cout << endl;
    assert( false );
    return CANCEL; // should not happen

  case CLOSE:

    if( local.addr != 0 )
    {
      delete (AtPeriodsLocalInfo *)local.addr;
      local.setAddr(Address(0));
    }
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
  Alpha resultUnit(true);
  Periods *periods;
  Interval<Instant> interval;
  bool foundUnit = false;

  switch( message )
  {
  case OPEN:
    localinfo = new AtPeriodsLocalInfoUS;
    localinfo->pWord = args[1];
    localinfo->j = 0;                              // init interval counter
    qp->Open( args[0].addr );                      // open stream of units
    qp->Request( args[0].addr, localinfo->uWord ); // request first unit
    if ( !( qp->Received( args[0].addr) ) ){
      localinfo->uWord.addr = 0;
      result.addr = 0;
      return CANCEL;
    }
    local.setAddr(localinfo);                    // pass up link to localinfo
    return 0;

    case REQUEST:
      if ( local.addr == 0 )
        return CANCEL;
      localinfo = (AtPeriodsLocalInfoUS *) local.addr; // restore local data
      if ( localinfo->uWord.addr == 0 ) { result.addr = 0; return CANCEL; }
      unit = (Alpha *) localinfo->uWord.addr;
      if ( localinfo->pWord.addr == 0 ) { result.addr = 0; return CANCEL; }
      periods = (Periods *) localinfo->pWord.addr;

      if( !periods->IsDefined() || periods->IsEmpty()       )
        return CANCEL;

      // search for a pair of overlapping unit/interval:
      while (1){
        if ( localinfo->j == periods->GetNoComponents() ){// redo first interval
          localinfo->j = 0;
          unit->DeleteIfAllowed();                // delete original unit?
          localinfo->uWord.addr = 0;
          foundUnit = false;
          while(!foundUnit){
            qp->Request(args[0].addr, localinfo->uWord);  // get new unit
            if( qp->Received( args[0].addr ) )
              unit = (Alpha *) localinfo->uWord.addr;
            else {
              localinfo->uWord.addr = 0;
              result.addr = 0;
              return CANCEL;
            }   // end of unit stream
            foundUnit = unit->IsDefined();
          }
        }
        periods->Get(localinfo->j, interval);       // get an interval
        if (    !( interval.Before( unit->timeInterval ) )
                  && !( unit->timeInterval.Before( interval) ) )
          break;                           // found candidate, break while
        localinfo->j++;                             // next interval, loop
      }

      // We have an interval overlapping the unit's interval now
      // Return unit restricted to overlapping part of both intervals
      if (!unit->timeInterval.Intersects( interval) ){ // This may not happen!
        cout << __FILE__ << __LINE__ << __PRETTY_FUNCTION__
             << ": Intervals do not overlap, but should do so:" << endl;
        cout << "  Unit's timeInterval = ";
        TUPrintTimeInterval(unit->timeInterval);
        cout << endl << "  Current Period's interval = ";
        TUPrintTimeInterval(interval);
        cout << endl;
        assert(false);
      }
      unit->AtInterval( interval, resultUnit); // intersect unit and interval
      aux = new Alpha( resultUnit );
      result.setAddr( aux );
      localinfo->j++;                           // increase interval counter
      return YIELD;

  case CLOSE:
    if ( local.addr != 0 )
      {
        qp->Close( args[0].addr );
        localinfo = (AtPeriodsLocalInfoUS *) local.addr;
        if ( localinfo->uWord.addr != 0 )
          {
            unit = (Alpha *) localinfo->uWord.addr;
            unit->DeleteIfAllowed();   // delete remaining original unit
          }
        delete (AtPeriodsLocalInfoUS *)localinfo;
        local.setAddr(Address(0));
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

Signatures

----
    For T in {bool, int, real, string, point, region}:

      (       uT) --> iT
      (stream uT) --> iT

----


*/
ListExpr
UnitTypeMapIntime( ListExpr args )
{
  ListExpr t;

  if ( nl->ListLength( args ) == 1 )
    {
      if (nl->IsAtom(nl->First(args)))
        t = nl->First( args );
      else if (nl->ListLength(nl->First(args))==2 &&
               nl->IsEqual(nl->First(nl->First(args)), Symbol::STREAM()))
        t = nl->Second(nl->First(args));
      else
        {
          ErrorReporter::ReportError
            ("Operator initial/final expects a (stream T)"
             "for T in {bool,int,real,string,point,region}");
          return nl->SymbolAtom( Symbol::TYPEERROR() );
        }

      if( nl->IsEqual( t, UBool::BasicType() ) )
        return nl->SymbolAtom( IBool::BasicType() );

      if( nl->IsEqual( t, UInt::BasicType() ) )
        return nl->SymbolAtom( IInt::BasicType() );

      if( nl->IsEqual( t, UReal::BasicType() ) )
        return nl->SymbolAtom( IReal::BasicType() );

      if( nl->IsEqual( t, UPoint::BasicType() ) )
        return nl->SymbolAtom( IPoint::BasicType() );

      if( nl->IsEqual( t, UString::BasicType() ) )
        return nl->SymbolAtom( IString::BasicType() );

      if( nl->IsEqual( t, URegion::BasicType() ) )
        return nl->SymbolAtom( IRegion::BasicType() );
    }
  else
    ErrorReporter::ReportError
      ("Operator initial/final expects a list of length one, "
       "containing a value of one type 'T' with T in "
       "{bool,int,real,string,point,region}");
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

/*
5.10.2 Value Mapping for ~initial~ and ~final~

*/

// first come the value mappings for (UNIT) argument
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

  Unit *unit = ((Unit*)args[0].addr);
  Intime<Alpha> *res = ((Intime<Alpha>*)result.addr);

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

//now, we give the value mappings for (stream UNIT) argument
// Mode=0 for initial, Mode=1 for final
template <class Unit, class Alpha, int Mode>
int MappingUnitStreamInstantFinal( Word* args, Word& result, int message,
                                   Word& local, Supplier s )
{
  assert(Mode>=0 && Mode<=1);
  result = qp->ResultStorage( s );
  Word elem;
  Unit *U = 0, *SavedUnit = 0;
  Intime<Alpha> *I = ((Intime<Alpha>*)result.addr);

  qp->Open(args[0].addr);              // get first elem from stream
  qp->Request(args[0].addr, elem);     // get first elem from stream
  while ( qp->Received(args[0].addr) ) // there is a element from the stream
    {
      U = (Unit*) elem.addr;
      if ( U->IsDefined() )
        {
          if (SavedUnit == 0)
            SavedUnit = U;
          else
            {
              if(Mode == 0)
                { // initial-mode
                  if ( U->timeInterval.start < SavedUnit->timeInterval.start)
                    {
                      SavedUnit->DeleteIfAllowed();
                      SavedUnit = U;
                    }
                  else
                    U->DeleteIfAllowed();
                }
              else // (Mode == 1)
                { // final-mode
                  if ( U->timeInterval.end > SavedUnit->timeInterval.end)
                    {
                      SavedUnit->DeleteIfAllowed();
                      SavedUnit = U;
                    }
                  else
                    {
                      U->DeleteIfAllowed();
                    }
                }
            }
        }
      else
        U->DeleteIfAllowed();
      qp->Request(args[0].addr, elem); // get next stream elem
    }
  qp->Close(args[0].addr); // close the stream

  // create and return the result
  if (SavedUnit == 0)
    I->SetDefined(false);
  else
    if(Mode == 0)
      { // initial-mode
        SavedUnit->TemporalFunction
          ( SavedUnit->timeInterval.start, I->value, true );
        I->instant.CopyFrom( &SavedUnit->timeInterval.start );
        I->SetDefined( true );
        SavedUnit->DeleteIfAllowed();
      }
    else // (Mode == 1)
      { // final-mode
        SavedUnit->TemporalFunction
          ( SavedUnit->timeInterval.end, I->value, true );
        I->instant.CopyFrom( &SavedUnit->timeInterval.end );
        I->SetDefined( true );
        SavedUnit->DeleteIfAllowed();
      }
  return 0;
}


/*
5.10.3 Specification for operator ~initial~ and ~final~

*/
const string
TemporalSpecInitial  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(       uT) -> iT\n"
  "(stream uT) -> iT\n"
  "(T in {bool, int, real, string, point, region})</text--->"
  "<text>initial( _ )</text--->"
  "<text>From a unit type (or a stream of units), get the "
  "intime value corresponding to the (overall) initial instant.</text--->"
  "<text>initial( upoint1 )</text---> ) )";

const string
TemporalSpecFinal  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>)       uT) -> iT\n"
  "(stream uT) -> iT\n"
  "(T in {bool, int, real, string, point, region})</text--->"
  "<text>final( _ )</text--->"
  "<text>get the intime value corresponding "
  "to the (overall) final instant of the (stream of) unit.</text--->"
  "<text>final( upoint1 )</text---> ) )";

/*
5.10.4 Selection Function of operator ~initial~ and ~final~

Using ~UnitCombinedUnitStreamSelect~.

*/

ValueMapping temporalunitinitialmap[] = {
  MappingUnitInitial<UBool, CcBool>,
  MappingUnitInitial<UInt, CcInt>,
  MappingUnitInitial<UReal, CcReal>,
  MappingUnitInitial<UPoint, Point>,
  MappingUnitInitial<UString, CcString>,
  MappingUnitInitial<URegion, Region>,
  MappingUnitStreamInstantFinal<UBool, CcBool, 0>,
  MappingUnitStreamInstantFinal<UInt, CcInt, 0>,
  MappingUnitStreamInstantFinal<UReal, CcReal, 0>,
  MappingUnitStreamInstantFinal<UPoint, Point, 0>,
  MappingUnitStreamInstantFinal<UString, CcString, 0>,
  MappingUnitStreamInstantFinal<URegion, Region, 0>
};

ValueMapping temporalunitfinalmap[] = {
  MappingUnitFinal<UBool, CcBool>,
  MappingUnitFinal<UInt, CcInt>,
  MappingUnitFinal<UReal, CcReal>,
  MappingUnitFinal<UPoint, Point>,
  MappingUnitFinal<UString, CcString>,
  MappingUnitFinal<URegion, Region>,
  MappingUnitStreamInstantFinal<UBool, CcBool, 1>,
  MappingUnitStreamInstantFinal<UInt, CcInt, 1>,
  MappingUnitStreamInstantFinal<UReal, CcReal, 1>,
  MappingUnitStreamInstantFinal<UPoint, Point, 1>,
  MappingUnitStreamInstantFinal<UString, CcString, 1>,
  MappingUnitStreamInstantFinal<URegion, Region, 1>
};

/*
5.10.5  Definition of operator ~initial~ and ~final~

*/
Operator temporalunitinitial
(
 "initial",
 TemporalSpecInitial,
 12,
 temporalunitinitialmap,
 UnitCombinedUnitStreamSelect,
 UnitTypeMapIntime );

Operator temporalunitfinal
(
 "final",
 TemporalSpecFinal,
 12,
 temporalunitfinalmap,
 UnitCombinedUnitStreamSelect,
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

    if( nl->IsEqual( arg2, Instant::BasicType() ) ||
        nl->IsEqual( arg2, Periods::BasicType() ) )
    {
      if( nl->IsEqual( arg1, UBool::BasicType() )  ||
          nl->IsEqual( arg1, UInt::BasicType() )   ||
          nl->IsEqual( arg1, UReal::BasicType() )  ||
          nl->IsEqual( arg1, UPoint::BasicType())  ||
          nl->IsEqual( arg1, UString::BasicType()) ||
          nl->IsEqual( arg1, URegion::BasicType())    )

        return nl->SymbolAtom( CcBool::BasicType() );
    }
  }
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

/*
5.11.2 Value Mapping for ~present~

*/

template <class Unit>
int MappingUnitPresent_i( Word* args, Word& result, int message,
                          Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  Unit *m = ((Unit*)args[0].addr);
  Instant* inst = ((Instant*)args[1].addr);
  Instant t1 = *inst;

  if ( !inst->IsDefined() || !m->IsDefined() )
    ((CcBool *)result.addr)->Set( false, false );
  else
    ((CcBool *)result.addr)->Set( true, m->timeInterval.Contains(t1) );
  return 0;
}

template <class Unit>
int MappingUnitPresent_p( Word* args, Word& result, int message,
                          Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  Unit *m = ((Unit*)args[0].addr);
  Periods* periods = ((Periods*)args[1].addr);

  if ( !m->IsDefined() || !periods->IsDefined() || periods->IsEmpty() )
  {
    ((CcBool *)result.addr)->Set( false, false );
    return 0;
  }

  // create a periods containing the smallest superinterval
  // of all intervals within the periods value.
  Periods deftime( 1 );
  deftime.Clear();
  deftime.SetDefined( true );
  deftime.StartBulkLoad();
  deftime.Add( m->timeInterval );
  deftime.EndBulkLoad( false );
  ((CcBool *)result.addr)->Set( true, periods->Intersects( deftime ) );
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
" given instant or period. For an empty or undefines periods value, "
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

  if( nl->SymbolValue( arg1 ) == UBool::BasicType() &&
      nl->SymbolValue( arg2 ) == Instant::BasicType() )
    return 0;

  if( nl->SymbolValue( arg1 ) == UInt::BasicType() &&
     nl->SymbolValue( arg2 ) == Instant::BasicType() )
    return 1;

  if( nl->SymbolValue( arg1 ) == UReal::BasicType() &&
      nl->SymbolValue( arg2 ) == Instant::BasicType() )
    return 2;

  if( nl->SymbolValue( arg1 ) == UPoint::BasicType() &&
      nl->SymbolValue( arg2 ) == Instant::BasicType() )
    return 3;

  if( nl->SymbolValue( arg1 ) == UString::BasicType() &&
      nl->SymbolValue( arg2 ) == Instant::BasicType() )
    return 4;

  if( nl->SymbolValue( arg1 ) == URegion::BasicType() &&
      nl->SymbolValue( arg2 ) == Instant::BasicType() )
    return 5;

  // periods versions:

  if( nl->SymbolValue( arg1 ) == UBool::BasicType() &&
      nl->SymbolValue( arg2 ) == Periods::BasicType() )
    return 6;

  if( nl->SymbolValue( arg1 ) == UInt::BasicType() &&
      nl->SymbolValue( arg2 ) == Periods::BasicType() )
    return 7;

  if( nl->SymbolValue( arg1 ) == UReal::BasicType() &&
      nl->SymbolValue( arg2 ) == Periods::BasicType() )
    return 8;

  if( nl->SymbolValue( arg1 ) == UPoint::BasicType() &&
      nl->SymbolValue( arg2 ) == Periods::BasicType() )
    return 9;

  if( nl->SymbolValue( arg1 ) == UString::BasicType() &&
      nl->SymbolValue( arg2 ) == Periods::BasicType() )
    return 10;

  if( nl->SymbolValue( arg1 ) == URegion::BasicType() &&
      nl->SymbolValue( arg2 ) == Periods::BasicType() )
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

    if( ((nl->IsEqual( arg1, UPoint::BasicType() )
      && nl->IsEqual( arg2, Point::BasicType() ))) )
      return nl->SymbolAtom( CcBool::BasicType() );
    if( ((nl->IsEqual( arg1, UInt::BasicType() )
      && nl->IsEqual( arg2, CcInt::BasicType() ))) )
      return nl->SymbolAtom( CcBool::BasicType() );
    if( ((nl->IsEqual( arg1, UBool::BasicType() )
      && nl->IsEqual( arg2, CcBool::BasicType() ))) )
      return nl->SymbolAtom( CcBool::BasicType() );
    if( ((nl->IsEqual( arg1, UReal::BasicType() )
      && nl->IsEqual( arg2, CcReal::BasicType() ))) )
      return nl->SymbolAtom( CcBool::BasicType() );
    if( ((nl->IsEqual( arg1, UString::BasicType() )
      && nl->IsEqual( arg2, CcString::BasicType() ))) )
      return nl->SymbolAtom( CcBool::BasicType() );
    if( ((nl->IsEqual( arg1, URegion::BasicType() )
      && nl->IsEqual( arg2, Point::BasicType() ))) )
      return nl->SymbolAtom( CcBool::BasicType() );
    if( ((nl->IsEqual( arg1, UPoint::BasicType() )
      && nl->IsEqual( arg2, Region::BasicType() ))) )
      return nl->SymbolAtom( CcBool::BasicType() );
    if( ((nl->IsEqual( arg1, UPoint::BasicType() )
      && nl->IsEqual( arg2, Rectangle<2>::BasicType() ))) )
      return nl->SymbolAtom( CcBool::BasicType() );
  }
  return nl->SymbolAtom( Symbol::TYPEERROR() );
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
  else
    ((CcBool *)result.addr)->Set( true, m->Passes( *val ) );
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
TUPassesSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args ),
           arg2 = nl->Second( args );

  if( nl->SymbolValue( arg1 ) == UBool::BasicType() &&
      nl->SymbolValue( arg2 ) == CcBool::BasicType() )
    return 0;

  if( nl->SymbolValue( arg1 ) == UInt::BasicType() &&
      nl->SymbolValue( arg2 ) == CcInt::BasicType() )
    return 1;

  if( nl->SymbolValue( arg1 ) == UReal::BasicType() &&
      nl->SymbolValue( arg2 ) == CcReal::BasicType() )
    return 2;

  if( nl->SymbolValue( arg1 ) == UPoint::BasicType() &&
      nl->SymbolValue( arg2 ) == Point::BasicType() )
    return 3;

  if( nl->SymbolValue( arg1 ) == UString::BasicType() &&
      nl->SymbolValue( arg2 ) == CcString::BasicType() )
    return 4;

  if( nl->SymbolValue( arg1 ) == URegion::BasicType() &&
      nl->SymbolValue( arg2 ) == Region::BasicType() )
    return 5;

  if( nl->SymbolValue( arg1 ) == UPoint::BasicType() &&
      nl->SymbolValue( arg2 ) == Region::BasicType() )
    return 6;

  if( nl->SymbolValue( arg1 ) == UPoint::BasicType() &&
      nl->SymbolValue( arg2 ) == Rectangle<2>::BasicType() )
    return 7;

  return -1; // This point should never be reached
}

ValueMapping temporalunitpassesmap[] = {
  MappingUnitPasses<UBool, CcBool>,      //0
  MappingUnitPasses<UInt, CcInt>,        //1
  MappingUnitPasses<UReal, CcReal>,      //2
  MappingUnitPasses<UPoint, Point>,      //3
  MappingUnitPasses<UString, CcString>,  //4
  MappingUnitPasses<URegion, Region>,    //5
  MappingUnitPasses<UPoint, Region>,     //6
  MappingUnitPasses<UPoint, Rectangle<2> >};  //7

/*
5.12.5  Definition of operator ~passes~

*/
Operator temporalunitpasses( "passes",
                         TemporalSpecPasses,
                         8,
                         temporalunitpassesmap,
                         TUPassesSelect,
                         UnitBaseTypeMapBool);


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

      if( nl->IsEqual( arg1, MPoint::BasicType() ) )
        return nl->SymbolAtom( MPoint::BasicType() );
      if( nl->IsEqual( arg1, UPoint::BasicType() ) )
        return nl->SymbolAtom( UPoint::BasicType() );
    }
  return nl->SymbolAtom( Symbol::TYPEERROR() );
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
  input->MVelocity( *res );
  return 0;
}

int UnitPointVelocity(Word* args, Word& result, int message,
                      Word& local, Supplier s)
{
  result = (qp->ResultStorage( s ));
  UPoint *input = (UPoint*)args[0].addr;
  UPoint *res   = (UPoint*)result.addr;
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
"<text>velocity ( Obj ) </text--->"
"<text>describes the vector of the vectorial velocity "
"of the given temporal spatial object Obj (i.e. the "
"componentwise speed in unit/s a vector). An undefined argument yields an "
"undefined result value</text--->"
"<text>velocity (mpoint)</text---> ) )";

/*
5.16.4 Selection Function of operator ~velocity~

*/
int
VelocitySelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );

  if( nl->SymbolValue( arg1 ) == MPoint::BasicType())
    return 0;

  if( nl->SymbolValue( arg1 ) == UPoint::BasicType())
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

      if( nl->IsEqual( arg1, MReal::BasicType() ) )
        return nl->SymbolAtom( MBool::BasicType() );
      if( nl->IsEqual( arg1, UReal::BasicType() ) )
        return nl->SymbolAtom( UBool::BasicType() );
    }
  return nl->SymbolAtom( Symbol::TYPEERROR() );
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

  UReal uReal;
  CcBool b;

  res->Clear();
  if ( !value->IsDefined() )
    res->SetDefined(false);
  else
    {
      res->SetDefined(true);
      res->StartBulkLoad();
      for( int i = 0; i < value->GetNoComponents(); i++ )
        {
          value->Get( i, uReal ); // Load a real unit.
          // FALSE means in this case that a real unit describes a quadratic
          // polynomial. A derivation is possible and the operator returns TRUE.
          if (uReal.r == false)
            b.Set(true,true);
          else
            b.Set(true,false);

          UBool boolvalue(uReal.timeInterval,b);
          res->MergeAdd( boolvalue );
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
      res->SetDefined(true);
      res->timeInterval = uReal->timeInterval;

      if (uReal->r == false)
        b.Set(true,true);
      else
        b.Set(true,false);

      UBool boolvalue(uReal->timeInterval,b);
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

  if( nl->SymbolValue( arg1 ) == MReal::BasicType())
    return 0;

  if( nl->SymbolValue( arg1 ) == UReal::BasicType())
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

      if( nl->IsEqual( arg1, MReal::BasicType() ) )
        return nl->SymbolAtom( MReal::BasicType() );
      if( nl->IsEqual( arg1, UReal::BasicType() ) )
        return nl->SymbolAtom( UReal::BasicType() );
    }
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}


/*
5.18.2 Value Mapping for ~derivative~

*/
int MPointDerivative( Word* args, Word& result, int message,
                      Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  MReal* m = ((MReal*)args[0].addr);
  MReal* pResult = ((MReal*)result.addr);
  UReal unitin;
  UReal unitout(true);

  pResult->Clear();
  if( !m->IsDefined() ){
    pResult->SetDefined( false );
    return 0;
  }
  pResult->SetDefined( true );
  pResult->StartBulkLoad();
  for(int i=0;i<m->GetNoComponents();i++)
  {
      m->Get(i, unitin);
      assert( unitin.IsDefined() );
      if(!unitin.r)
      {
          unitout.a = 0.;
          unitout.b = 2*unitin.a;
          unitout.c = unitin.b;
          unitout.r = false;
          unitout.timeInterval = unitin.timeInterval;
          pResult->MergeAdd(unitout);
      }
  }
  pResult->EndBulkLoad( false );

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

Moved to ~StreamAlgebra~ and renamed it to ~feed~.

*/


/*
5.20 Operator ~suse~

Moved to ~StreamAlgebra~ and renamed it to ~use~ resp. ~use2~.

*/

/*
5.21 Operator ~distance~

The operator calculates the minimum distance between two units of base
types ~int~ or ~point~. The distance is always a non-negative ~ureal~ value.

----
    For T in {int, point}
    distance:    uT x    uT -> ureal
                 uT x     T -> ureal
                  T x    uT -> ureal
              ureal x ureal -> (stream ureal)
               real x ureal -> (stream ureal)
              ureal x  real -> (stream ureal)

----


5.21.1 Type mapping function for ~distance~

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
      return nl->SymbolAtom( Symbol::TYPEERROR() );
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
      return nl->SymbolAtom( Symbol::TYPEERROR() );
    }

  if( nl->IsEqual(first, UPoint::BasicType())
    && nl->IsEqual(second, UPoint::BasicType()) )
    {
      return nl->SymbolAtom(UReal::BasicType());
    }

  if( nl->IsEqual(first, UPoint::BasicType())
    && nl->IsEqual(second, Point::BasicType()) )
    {
      return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                               nl->OneElemList(nl->IntAtom(0)),
                               nl->SymbolAtom( UReal::BasicType() ));
    }

  if( nl->IsEqual(first, Point::BasicType())
    && nl->IsEqual(second, UPoint::BasicType()) )
    {
      return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                               nl->OneElemList(nl->IntAtom(1)),
                               nl->SymbolAtom( UReal::BasicType() ));
    }

  if( nl->IsEqual(first, UInt::BasicType())
    && nl->IsEqual(second, UInt::BasicType()) )
    {
      return nl->SymbolAtom(UReal::BasicType());
    }

  if( nl->IsEqual(first, UInt::BasicType())
    && nl->IsEqual(second, CcInt::BasicType()) )
    {
      return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                               nl->OneElemList(nl->IntAtom(0)),
                               nl->SymbolAtom( UReal::BasicType() ));
    }

  if( nl->IsEqual(first, CcInt::BasicType())
    && nl->IsEqual(second, UInt::BasicType()) )
    {
      return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                               nl->OneElemList(nl->IntAtom(1)),
                               nl->SymbolAtom( UReal::BasicType() ));
    }

    if( nl->IsEqual(first, UReal::BasicType())
      && nl->IsEqual(second, UReal::BasicType()) )
    {
      return  nl->TwoElemList(nl->SymbolAtom( Symbol::STREAM() ),
                              nl->SymbolAtom( UReal::BasicType() ));
    }

    if( nl->IsEqual(first, UReal::BasicType())
      && nl->IsEqual(second, CcReal::BasicType()) )
    {
      return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                               nl->OneElemList(nl->IntAtom(0)),
                               nl->TwoElemList(nl->SymbolAtom(
                               Symbol::STREAM() ),
                                        nl->SymbolAtom( UReal::BasicType() )));
    }

    if( nl->IsEqual(first, CcReal::BasicType())
      && nl->IsEqual(second, UReal::BasicType()) )
    {
      return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                               nl->OneElemList(nl->IntAtom(1)),
                               nl->TwoElemList(
                                        nl->SymbolAtom( Symbol::STREAM() ),
                                        nl->SymbolAtom( UReal::BasicType() )));
    }

  nl->WriteToString(outstr1, first);
  nl->WriteToString(outstr2, second);
  ErrorReporter::ReportError("Operator distance found wrong argument "
                             "configuration '" + outstr1 +
                             "' and '" + outstr2 + "'.");
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

/*
5.21.2 Value mapping for operator ~distance~
*/


/*
(a1) Value mapping for

---- (upoint upoint) -> ureal

----

*/

int TUDistance_UPoint_UPoint( Word* args, Word& result, int message,
                              Word& local, Supplier s )
{
  Interval<Instant> iv;

  //  Word a1, a2;
  UPoint *u1, *u2;
  result = qp->ResultStorage( s );
  UReal* res = (UReal*) result.addr;

  u1 = (UPoint*)(args[0].addr);
  u2 = (UPoint*)(args[1].addr);
  if (!u1->IsDefined() ||
      !u2->IsDefined() ||
      !u1->timeInterval.Intersects( u2->timeInterval ) )
    { // return undefined ureal
      res->SetDefined( false );
    }
  else
    { // get intersection of deftime intervals
#ifdef TUA_DEBUG
      cout << __PRETTY_FUNCTION__ << ":" << endl
           << "   iv1=" << TUPrintTimeInterval(u1->timeInterval) << endl
           << "   iv2=" << TUPrintTimeInterval(u2->timeInterval) << endl;
#endif
      u1->timeInterval.Intersection( u2->timeInterval, iv );
#ifdef TUA_DEBUG
      cout << __PRETTY_FUNCTION__ << ": iv="
           << TUPrintTimeInterval(iv) << endl;
#endif
      // calculate result
      u1->Distance( *u2, *res );
      res->SetDefined( true );
    }
  // pass on result
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
  result = qp->ResultStorage( s );
  UReal*  res = static_cast<UReal*>(result.addr);

  // get argument configuration
  int   argConfDescriptor2;
  argConfDescriptor2 = ((CcInt*)args[2].addr)->GetIntval();
  if (argConfDescriptor2 == 0) {
      theUPoint = args[0];
      thePoint  = args[1];
  } else if (argConfDescriptor2 == 1) {
      theUPoint = args[1];
      thePoint  = args[0];
  } else {
      cout << "\nWrong argument configuration in "
           << __PRETTY_FUNCTION__ << ": argConfDescriptor2="
           << argConfDescriptor2 << endl;
      assert( false );
      return 0;
  }
  UPoint* up  = static_cast<UPoint*>(theUPoint.addr);
  Point*  p   = static_cast<Point*>(thePoint.addr);

  up->Distance( *p, *res);
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
  double c1, c2, c;

  result = qp->ResultStorage( s );
  UReal* res = static_cast<UReal*>(result.addr);
  UInt*  u1  = static_cast<UInt*>(args[0].addr);
  UInt*  u2  = static_cast<UInt*>(args[1].addr);

  if (!u1->IsDefined() ||
      !u2->IsDefined() ||
      !u1->timeInterval.Intersects( u2->timeInterval ) )
    { // return undefined ureal
      res->SetDefined( false );
    }
  else
    { // get intersection of deftime intervals
      u1->timeInterval.Intersection( u2->timeInterval, iv );

      // calculate  result
      // (as the result is constant, no translation step is required)
      c1 = static_cast<double>(u2->constValue.GetIntval());
      c2 = static_cast<double>(u2->constValue.GetIntval());
      c = fabs(c1 - c2);
      *res = UReal(iv, 0, 0, c, false);
      res->SetDefined( true );
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
(c2) value mapping for

---- ((ureal ureal) -> (stream ureal))

----

*/
struct TUDistanceLocalInfo
{
  bool finished;
  int NoOfResults;
  int NoOfResultsDelivered;
  vector<UReal> resultVector;
};

int TUDistance_UReal_UReal( Word* args, Word& result, int message,
                            Word& local, Supplier s )
{
  UReal  *u1, *u2, utemp(true);
  TUDistanceLocalInfo *localinfo = 0;

  result = qp->ResultStorage( s );

  switch( message )
  {
    case OPEN:

      u1 = (UReal*)(args[0].addr);
      u2 = (UReal*)(args[1].addr);

      localinfo = new TUDistanceLocalInfo;
      local.setAddr(localinfo);
      localinfo->finished = true;
      localinfo->NoOfResults = 0;
      localinfo->NoOfResultsDelivered = 0;
      localinfo->resultVector.clear();

      if (!u1->IsDefined() ||
          !u2->IsDefined() ||
           u1->r           ||
           u2->r
         )
      { // return empty stream
        return 0;
      }
      else
      { // calculate results
        localinfo->NoOfResults = u1->Distance(*u2, localinfo->resultVector);
        localinfo->finished = (localinfo->NoOfResults <= 0);
        return 0;
      }

    case REQUEST:
      if(local.addr == 0)
        return CANCEL;
      localinfo = (TUDistanceLocalInfo*) local.addr;
      if( localinfo->finished ||
          localinfo->NoOfResultsDelivered >= localinfo->NoOfResults )
      {
        localinfo->finished = true;
        return CANCEL;
      }
      result.setAddr(
          localinfo->resultVector[localinfo->NoOfResultsDelivered].Clone() );
      localinfo->NoOfResultsDelivered++;
      return YIELD;

    case CLOSE:
      if(local.addr != 0)
      {
         localinfo = (TUDistanceLocalInfo*) local.addr;
        delete localinfo;
        local.setAddr(Address(0));
      }
      return 0;
  }
  return 0;
}

/*
(c2) value mapping for

---- ((ureal real) -> (stream ureal)) and ((real ureal) -> (stream ureal))

----

*/
int TUDistance_UReal_Real( Word* args, Word& result, int message,
                         Word& local, Supplier s )
{
  Word  ii, ui;
  int   argConfDescriptor2;
  UReal  *u, utemp(true);
  CcReal *i;
  TUDistanceLocalInfo *localinfo = 0;

  result = qp->ResultStorage( s );

  switch( message )
  {
    case OPEN:

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
          << "'TUDistance_UReal_Real'. argConfDescriptor2="
          << argConfDescriptor2 << endl;
      return 0;
    }

    u = (UReal*)(ui.addr);
    i = (CcReal*)(ii.addr);

    localinfo = new TUDistanceLocalInfo;
    local.setAddr(localinfo);
    localinfo->finished = true;
    localinfo->NoOfResults = 0;
    localinfo->NoOfResultsDelivered = 0;
    localinfo->resultVector.clear();

    if (!u->IsDefined() ||
        !i->IsDefined() ||
        u->r
       )
    { // return empty stream
      return 0;
    }
    else
    { // calculate  result
      utemp =
        UReal(u->timeInterval, u->a, u->b, (u->c - i->GetRealval() ), false);
      localinfo->NoOfResults = utemp.Abs(localinfo->resultVector);
      localinfo->finished = (localinfo->NoOfResults <= 0);
      return 0;
    }

    case REQUEST:
      if(local.addr != 0)
        localinfo = (TUDistanceLocalInfo*) local.addr;
      else
        return CANCEL;
      if( localinfo->finished ||
          localinfo->NoOfResultsDelivered >= localinfo->NoOfResults )
      {
        localinfo->finished = true;
        return CANCEL;
      }
      result.setAddr(
          localinfo->resultVector[localinfo->NoOfResultsDelivered].Clone() );
      localinfo->NoOfResultsDelivered++;
      return YIELD;

    case CLOSE:
      if(local.addr != 0)
      {
        localinfo = (TUDistanceLocalInfo*) local.addr;
        delete localinfo;
        local.setAddr(Address(0));
      }
      return 0;
  }
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
  "( T uT) -> ureal\n"
  "(ureal ureal) -> (stream ureal)\n"
  "(ureal  real) -> (stream ureal)\n"
  "( real ureal) -> (stream ureal)</text--->"
  "<text>distance( _, _)</text--->"
  "<text>Calculates the distance of both arguments, "
  "whereof at least one is a unittype, "
  "as a 'ureal' value. For the ureal signatures, all "
  "radix-flags must be FALSE, otherwise the result is "
  "empty.</text--->"
  "<text>distance(upoint1,point1)</text--->"
  ") )";

/*
5.21.4 Selection Function of operator ~distance~

*/

ValueMapping temporalunitdistancemap[] =
  {
    TUDistance_UPoint_UPoint,  // 0
    TUDistance_UPoint_Point,
    TUDistance_UInt_UInt,
    TUDistance_UInt_Int,
    TUDistance_UReal_UReal,
    TUDistance_UReal_Real      // 5
  };

int temporalunitDistanceSelect( ListExpr args )
{
  ListExpr first  = nl->First(args);
  ListExpr second = nl->Second(args);

  if( nl->IsEqual(first, UPoint::BasicType())
    && nl->IsEqual(second, UPoint::BasicType()) )
    return 0;

  else if( nl->IsEqual(first, UPoint::BasicType())
    && nl->IsEqual(second, Point::BasicType()) )
    return 1;

  else if( nl->IsEqual(first, Point::BasicType())
    && nl->IsEqual(second, UPoint::BasicType()) )
    return 1;

  else if( nl->IsEqual(first, UInt::BasicType())
    && nl->IsEqual(second, UInt::BasicType()) )
    return 2;

  else if( nl->IsEqual(first, UInt::BasicType())
    && nl->IsEqual(second, CcInt::BasicType()) )
    return 3;

  else if( nl->IsEqual(first, CcInt::BasicType())
    && nl->IsEqual(second, UInt::BasicType()) )
    return 3;

  else if( nl->IsEqual(first, UReal::BasicType())
    && nl->IsEqual(second, UReal::BasicType()) )
    return 4;

  else if( nl->IsEqual(first, UReal::BasicType())
    && nl->IsEqual(second, CcReal::BasicType()) )
    return 5;

  else if( nl->IsEqual(first, CcReal::BasicType())
    && nl->IsEqual(second, UReal::BasicType()) )
    return 5;

  else
    cout << "\nERROR in temporalunitDistanceSelect!" << endl;

  return -1;
}

/*
5.21.5 Definition of operator ~distance~

*/

Operator temporalunitdistance( "distance",
                               TemporalSpecDistance,
                               6,
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

      if( nl->IsEqual( arg1, UBool::BasicType() ) )
        return nl->SymbolAtom( UBool::BasicType() );
      if( nl->IsEqual( arg1, UInt::BasicType() ) )
        return nl->SymbolAtom( UInt::BasicType() );
      if( nl->IsEqual( arg1, UString::BasicType() ) )
        return nl->SymbolAtom( UString::BasicType() );
      // for ureal, atmax/atmin will return a stream of ureals!
      if( nl->IsEqual( arg1, UReal::BasicType() ) )
        return nl->TwoElemList(nl->SymbolAtom( Symbol::STREAM() ),
                               nl->SymbolAtom( UReal::BasicType() ));
    }
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}
/*
5.22.2 Value mapping for operator ~atmax~

*/

struct AtExtrURealLocalInfo
{
  int NoOfResults;
  int ResultsDelivered;
  vector<UReal> resultUnitVector;
};


int atmaxUReal( Word* args, Word& result, int message,
                Word& local, Supplier s )
{
  AtExtrURealLocalInfo *sli;
  UReal                *ureal;

  result = qp->ResultStorage( s );

  switch (message)
    {
    case OPEN :

      ureal = (UReal*)(args[0].addr);
#ifdef TUA_DEBUG
        cout << "  Argument ureal value: " << TUPrintUReal(ureal) << endl
             << "  1" << endl;
#endif
      sli = new AtExtrURealLocalInfo;
      sli->resultUnitVector.clear();
      sli->NoOfResults = 0;
      sli->ResultsDelivered = 0;
      local.setAddr(sli);

      if ( !ureal->IsDefined() )
        { // ureal undefined
          // -> return empty stream
          sli->NoOfResults = 0;
#ifdef TUA_DEBUG
          cout << "       ureal undef: no solution" << endl;
#endif
          return 0;
        }
      sli->NoOfResults = ureal->AtMax(sli->resultUnitVector);
      return 0;

    case REQUEST :

      if (local.addr == 0)
        return CANCEL;
      sli = (AtExtrURealLocalInfo*) local.addr;

      if (sli->NoOfResults <= sli->ResultsDelivered)
        return CANCEL;

      result.setAddr( sli->resultUnitVector[sli->ResultsDelivered].Clone() );
#ifdef TUA_DEBUG
        cout << "    delivered result[" << sli->ResultsDelivered+1
             << "/" << sli->NoOfResults<< "]="
             << TUPrintUReal((UReal*)(result.addr))
             << endl;
#endif
      sli->ResultsDelivered++;
      return YIELD;

    case CLOSE :

      if (local.addr != 0)
        {
          sli = (AtExtrURealLocalInfo*) local.addr;
//           for(unsigned int i=0; i< sli->resultUnitVector.size(); i++)
//             sli->resultUnitVector[i].DeleteIfAllowed();
          delete sli;
          local.setAddr(Address(0));
        }
      return 0;

    } // end switch
  return 0;   // should not be reached
}

template<class T>
int atmaxUConst( Word* args, Word& result, int message,
                 Word& local, Supplier s )
{
  ConstTemporalUnit<T>* arg = static_cast<ConstTemporalUnit<T>*>(args[0].addr);
  result = qp->ResultStorage( s );
  // This operator is not very interesting. It implements
  // the atmax operator for constant unit types, like uint, ustring or ubool.
  // In fact, it returns just a copy of the argument.
  ConstTemporalUnit<T>* res = static_cast<ConstTemporalUnit<T>*>(result.addr);
  res->CopyFrom(arg);
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

      if( nl->IsEqual( arg1, UBool::BasicType() ) )
        return 0;
      if( nl->IsEqual( arg1, UInt::BasicType() ) )
        return 1;
      if( nl->IsEqual( arg1, UString::BasicType() ) )
        return 2;
      if( nl->IsEqual( arg1, UReal::BasicType() ) )
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

int atminUReal( Word* args, Word& result, int message,
                Word& local, Supplier s )
{
  AtExtrURealLocalInfo *sli;
  UReal                *ureal;

  result = qp->ResultStorage( s );

  switch (message)
    {
    case OPEN :

      ureal = (UReal*)(args[0].addr);
#ifdef TUA_DEBUG
        cout << "  Argument ureal value: " << TUPrintUReal(ureal) << endl
             << "  1" << endl;
#endif
      sli = new AtExtrURealLocalInfo;
      sli->resultUnitVector.clear();
      sli->NoOfResults = 0;
      sli->ResultsDelivered = 0;
      local.setAddr(sli);

      if ( !ureal->IsDefined() )
        { // ureal undefined
          // -> return empty stream
          sli->NoOfResults = 0;
#ifdef TUA_DEBUG
          cout << "       ureal undef: no solution" << endl;
#endif
          return 0;
        }
      sli->NoOfResults = ureal->AtMin(sli->resultUnitVector);
      return 0;

    case REQUEST :

      if (local.addr == 0)
        return CANCEL;
      sli = (AtExtrURealLocalInfo*) local.addr;

      if (sli->NoOfResults <= sli->ResultsDelivered)
        return CANCEL;

      result.setAddr( sli->resultUnitVector[sli->ResultsDelivered].Clone() );
#ifdef TUA_DEBUG
        cout << "    delivered result[" << sli->ResultsDelivered+1
             << "/" << sli->NoOfResults<< "]="
             << TUPrintUReal((UReal*)(result.addr))
             << endl;
#endif
      sli->ResultsDelivered++;
      return YIELD;

    case CLOSE :

      if (local.addr != 0)
      {
        sli = (AtExtrURealLocalInfo*) local.addr;
//           for(unsigned int i=0; i< sli->resultUnitVector.size(); i++)
//             sli->resultUnitVector[i].DeleteIfAllowed();
        delete sli;
        local.setAddr(Address(0));
       }
      return 0;

    } // end switch
  return 0;   // should not be reached
}

template<class T>
int atminUConst( Word* args, Word& result, int message,
                 Word& local, Supplier s )
{
  ConstTemporalUnit<T>* arg = static_cast<ConstTemporalUnit<T>*>(args[0].addr);
  result = qp->ResultStorage( s );
  // This operator is not very interesting. It implements
  // the atmin operator for constant unit types, like uint, ustring or ubool.
  // In fact, it returns just a copy of the argument.
  ConstTemporalUnit<T>* res = static_cast<ConstTemporalUnit<T>*>(result.addr);
  res->CopyFrom(arg);
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

Moved to ~StreamAlgebra~ and remaned it to ~aggregateS~

*/

/*
5.25 Operator ~abs~

The operator returns the absolute function derived from the
input ureal or uint.

*/

/*
5.25.1 Type mapping function for ~abs~

The operator has signature

----

  abs: ureal --> (stream ureal)
  abs:  uint --> uint

----

*/

ListExpr TU_TM_Abs( ListExpr args )
{
  ListExpr first;
  string outstr1, outstr2;

  if ( nl->IsAtom( args ) || nl->ListLength( args ) != 1 )
  {
    nl->WriteToString(outstr1, args);
    ErrorReporter::ReportError("Operator abs expects a list of "
        "length one, but gets '" + outstr1 +
        "'.");
    return nl->SymbolAtom( Symbol::TYPEERROR() );
  }

  first = nl->First(args);

  if( nl->IsEqual(first, UInt::BasicType()) )
  {
    return nl->SymbolAtom(UInt::BasicType());
  }

  if( nl->IsEqual(first, UReal::BasicType()) )
  {
    return  nl->TwoElemList(nl->SymbolAtom( Symbol::STREAM() ),
                            nl->SymbolAtom( UReal::BasicType() ));
  }

  nl->WriteToString(outstr1, first);
  ErrorReporter::ReportError("Operator abs found wrong argument "
      + outstr1 + "'.");
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}


/*
5.25.2 Value mapping for operator ~abs~

*/
// value mapping for uint --> uint
int TU_VM_Abs_UInt( Word* args, Word& result, int message,
                    Word& local, Supplier s )
{
  Word a1;
  UInt *u1;
  long c1, c;

  result = qp->ResultStorage( s );

  a1 = args[0];

  u1 = (UInt*)(a1.addr);

  if ( !u1->IsDefined() )
  { // return undefined ureal
    ((UInt*)(result.addr))->SetDefined( false );
  }
  else
  {
    c1 = (long) u1->constValue.GetIntval();
    c = abs(c1);
    ((UInt*)(result.addr))->timeInterval = u1->timeInterval;
    ((UInt*)(result.addr))->SetDefined( true );
    ((UInt*)(result.addr))->constValue.Set(true,c);
  }
  return 0;
}

struct TUAbsLocalInfo
{
  bool finished;
  int  NoOfResults;
  int  NoOfResultsDelivered;
  vector<UReal> resultVector;
};

// value mapping for ureal --> (stream ureal)
int TU_VM_Abs_UReal( Word* args, Word& result, int message,
                     Word& local, Supplier s )
{
  TUAbsLocalInfo *sli;
  Word u1;
  UReal *uv1;
  Interval<Instant> iv;

  switch( message )
  {
    case OPEN:

      sli = new TUAbsLocalInfo;
      local.setAddr(sli);
      sli->resultVector.clear();
      sli->finished = true;
      sli->NoOfResults = 0;
      sli->NoOfResultsDelivered = 0;

      u1 = args[0];
      uv1 = (UReal*) (u1.addr);

      if ( uv1->IsDefined() )
      {
        sli->NoOfResults = uv1->Abs(sli->resultVector);
        sli->finished = ( sli->NoOfResults <= 0 );
      }
      return 0;

    case REQUEST:

      if(local.addr == 0)
      {
        return CANCEL;
      }
      sli = (TUAbsLocalInfo*) local.addr;
      if(sli->finished)
      {
        return CANCEL;
      }
      if(sli->NoOfResultsDelivered < sli->NoOfResults)
      {
        result.setAddr(
            sli->resultVector[sli->NoOfResultsDelivered].Clone() );
        sli->NoOfResultsDelivered++;
        return YIELD;
      }
      sli->finished = true;
      return CANCEL;

    case CLOSE:

      if (local.addr != 0)
      {
        sli = (TUAbsLocalInfo*) local.addr;
        delete sli;
        local.setAddr(Address(0));
      }
      return 0;
  } // end switch

  return 0;
}


/*
5.24.3 Specification for operator ~abs~

*/
const string TU_Spec_Abs  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>uint -> uint\n"
    "ureal -> (stream ureal)</text--->"
    "<text>abs( _ )</text--->"
    "<text>Return the argument's absolute value.</text--->"
    "<text>query speed(mp1)</text---> ) )";


/*
5.25.4 Selection Function of operator ~abs~

*/
  int TU_Select_Abs( ListExpr args )
{
  ListExpr arg1 = nl->First( args );

  if( nl->SymbolValue( arg1 ) == UInt::BasicType() )
    return 0;

  if( nl->SymbolValue( arg1 ) == UReal::BasicType() )
    return 1;

  return -1; // This point should never be reached
}

ValueMapping temporalunit_abs_map[] = { TU_VM_Abs_UInt, TU_VM_Abs_UReal };

/*
5.25.5 Definition of operator ~abs~

*/
  Operator temporalunitabs( "abs",
                          TU_Spec_Abs,
                          2,
                          temporalunit_abs_map,
                          TU_Select_Abs,
                          TU_TM_Abs);


/*
5.26 Operator

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
OK         upoint x  region --> (stream upoint)
OK         region x  upoint --> (stream upoint)
OK         upoint x uregion --> (stream upoint)
OK        uregion x  upoint --> (stream upoint)

----

A. (T mT) [->] mT   and   (mT T) -> mT

B. (mT mT) [->] mT

The operator always returns a stream of units (to handle both, empty and set
valued results).

  1. For all types of arguments, we return an empty stream, if both 
timeIntervals don't overlap.

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
  string argstr1 = "", argstr2 = "";

  if( nl->ListLength( args ) == 2 )
    {
      arg1 = nl->First( args );
      arg2 = nl->Second( args );

      // First case: uT uT -> stream uT
      if (nl->Equal( arg1, arg2 ))
        {
          if( nl->IsEqual( arg1, UBool::BasicType() ) )
            return  nl->TwoElemList(nl->SymbolAtom( Symbol::STREAM() ),
                                    nl->SymbolAtom( UBool::BasicType() ));
          if( nl->IsEqual( arg1, UInt::BasicType() ) )
            return nl->TwoElemList(nl->SymbolAtom( Symbol::STREAM() ),
                                   nl->SymbolAtom( UInt::BasicType() ));
          if( nl->IsEqual( arg1, UReal::BasicType() ) )
            return nl->TwoElemList(nl->SymbolAtom( Symbol::STREAM() ),
                                   nl->SymbolAtom( UReal::BasicType() ));
          if( nl->IsEqual( arg1, UPoint::BasicType() ) )
            return nl->TwoElemList(nl->SymbolAtom( Symbol::STREAM() ),
                                   nl->SymbolAtom( UPoint::BasicType() ));
          if( nl->IsEqual( arg1, UString::BasicType() ) )
            return nl->TwoElemList(nl->SymbolAtom( Symbol::STREAM() ),
                                   nl->SymbolAtom( UString::BasicType() ));
        }

      // Second case: uT T -> stream uT
      if( nl->IsEqual( arg1, UBool::BasicType() )
        && nl->IsEqual( arg2, CcBool::BasicType()) )
        return  nl->TwoElemList(nl->SymbolAtom( Symbol::STREAM() ),
                                nl->SymbolAtom( UBool::BasicType() ));
      if( nl->IsEqual( arg1, UInt::BasicType() )
        && nl->IsEqual( arg2, CcInt::BasicType()) )
        return nl->TwoElemList(nl->SymbolAtom( Symbol::STREAM() ),
                               nl->SymbolAtom( UInt::BasicType() ));
      if( nl->IsEqual( arg1, UReal::BasicType() )
        && nl->IsEqual( arg2, CcReal::BasicType()) )
        return nl->TwoElemList(nl->SymbolAtom( Symbol::STREAM() ),
                               nl->SymbolAtom( UReal::BasicType() ));
      if( nl->IsEqual( arg1, UPoint::BasicType() )
        && nl->IsEqual( arg2, Point::BasicType()) )
        return nl->TwoElemList(nl->SymbolAtom( Symbol::STREAM() ),
                               nl->SymbolAtom( UPoint::BasicType() ));
      if( nl->IsEqual( arg1, UString::BasicType() )
        && nl->IsEqual( arg2, CcString::BasicType()) )
        return nl->TwoElemList(nl->SymbolAtom( Symbol::STREAM() ),
                               nl->SymbolAtom( UString::BasicType() ));

      // Third case: T uT -> stream uT
      if( nl->IsEqual( arg1, CcBool::BasicType() )
        && nl->IsEqual( arg2, UBool::BasicType()) )
        return  nl->TwoElemList(nl->SymbolAtom( Symbol::STREAM() ),
                                nl->SymbolAtom( UBool::BasicType() ));
      if( nl->IsEqual( arg1, CcInt::BasicType() )
        && nl->IsEqual( arg2, UInt::BasicType()) )
        return nl->TwoElemList(nl->SymbolAtom( Symbol::STREAM() ),
                               nl->SymbolAtom( UInt::BasicType() ));
      if( nl->IsEqual( arg1, CcReal::BasicType() )
        && nl->IsEqual( arg2, UReal::BasicType()) )
        return nl->TwoElemList(nl->SymbolAtom( Symbol::STREAM() ),
                               nl->SymbolAtom( UReal::BasicType() ));
      if( nl->IsEqual( arg1, Point::BasicType() )
        && nl->IsEqual( arg2, UPoint::BasicType()) )
        return nl->TwoElemList(nl->SymbolAtom( Symbol::STREAM() ),
                               nl->SymbolAtom( UPoint::BasicType() ));
      if( nl->IsEqual( arg1, CcString::BasicType() )
        && nl->IsEqual( arg2, UString::BasicType()) )
        return nl->TwoElemList(nl->SymbolAtom( Symbol::STREAM() ),
                               nl->SymbolAtom( UString::BasicType() ));

      // Fourth case: upoint line -> stream upoint
      if( nl->IsEqual( arg1, UPoint::BasicType() )
        && nl->IsEqual( arg2, Line::BasicType()) )
        return  nl->TwoElemList(nl->SymbolAtom( Symbol::STREAM() ),
                                nl->SymbolAtom( UPoint::BasicType() ));

      // Fifth case: line upoint -> stream upoint
      if( nl->IsEqual( arg1, Line::BasicType() )
        && nl->IsEqual( arg2, UPoint::BasicType()) )
        return  nl->TwoElemList(nl->SymbolAtom( Symbol::STREAM() ),
                                nl->SymbolAtom( UPoint::BasicType() ));

      // Sixth case: upoint uregion -> stream upoint
      if( nl->IsEqual( arg1, UPoint::BasicType() )
        && nl->IsEqual( arg2, URegion::BasicType()) )
        return  nl->TwoElemList(nl->SymbolAtom( Symbol::STREAM() ),
                                nl->SymbolAtom( UPoint::BasicType() ));

      // Eighth case: uregion upoint -> stream upoint
      if( nl->IsEqual( arg1, URegion::BasicType() )
        && nl->IsEqual( arg2, UPoint::BasicType()) )
        return  nl->TwoElemList(nl->SymbolAtom( Symbol::STREAM() ),
                                nl->SymbolAtom( UPoint::BasicType() ));

      // Ninth case: upoint region -> stream upoint
      if( nl->IsEqual( arg1, UPoint::BasicType() )
        && nl->IsEqual( arg2, Region::BasicType()) )
       return  nl->TwoElemList(nl->SymbolAtom( Symbol::STREAM() ),
                               nl->SymbolAtom( UPoint::BasicType() ));

      // Tenth case: region upoint -> stream upoint
      if( nl->IsEqual( arg1, Region::BasicType() )
        && nl->IsEqual( arg2, UPoint::BasicType()) )
       return  nl->TwoElemList(nl->SymbolAtom( Symbol::STREAM() ),
                               nl->SymbolAtom( UPoint::BasicType() ));

      // Error case:
      nl->WriteToString(argstr1, arg1);
      nl->WriteToString(argstr2, arg2);
      ErrorReporter::ReportError(
          "Operator intersection expects argumentlist (T,T), (uT,T), (T,uT), "
          "where T in {ubool, uint, ureal, ustring, upoint}\n or a combination "
          "of {upoint,line}, {upoint,uregion}, {upoint,region}.\n"
          "The passed arguments have types '"+ argstr1 +"' and '"
          + argstr2 + "'.");
    } else {
      // Error case:
      ErrorReporter::ReportError(
        "Operator intersection expects argumentlist (T,T), (uT,T), (T,uT), "
        "where T in {ubool, uint, ureal, ustring, upoint}\n or a combination of"
        " {upoint,line}, {upoint,uregion}, {upoint,region}.\n");
    }
    return nl->SymbolAtom(Symbol::TYPEERROR());

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

#ifdef TUA_DEBUG
        cout << "temporalUnitIntersection_CU_CU: received OPEN" << endl;
#endif
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
#ifdef TUA_DEBUG
          cout << "  iv=" << TUPrintTimeInterval( iv ) << endl;
#endif
          // store result
          (sli->resultValues[sli->NoOfResults]).setAddr( uv1->Clone() );
          ((T*)(sli->resultValues[sli->NoOfResults].addr))->timeInterval = iv;
          sli->NoOfResults++;
          sli->finished = false;
#ifdef TUA_DEBUG
          cout << "  added result" << endl;
#endif
        }// else: no result
      local.setAddr(sli);
#ifdef TUA_DEBUG
      cout << "temporalUnitIntersection_CU_CU: finished OPEN" << endl;
#endif

      return 0;

    case REQUEST:

#ifdef TUA_DEBUG
      cout << "temporalUnitIntersection_CU_CU: received REQUEST" << endl;
#endif
      if(local.addr == 0)
        {
#ifdef TUA_DEBUG
          cout << "temporalUnitIntersection_CU_CU: CANCEL (1)" << endl;
#endif
          return CANCEL;
        }
      sli = (TUIntersectionLocalInfo*) local.addr;
      if(sli->finished)
        {
#ifdef TUA_DEBUG
          cout << "temporalUnitIntersection_CU_CU: CANCEL (2)" << endl;
#endif
          return CANCEL;
        }
      if(sli->NoOfResultsDelivered < sli->NoOfResults)
        {
          result.setAddr( ((T*)
            (sli->resultValues[sli->NoOfResultsDelivered].addr))->Clone() );
          ((T*)(sli->resultValues[sli->NoOfResultsDelivered].addr))
            ->DeleteIfAllowed();
          sli->NoOfResultsDelivered++;
#ifdef TUA_DEBUG
          cout << "temporalUnitIntersection_CU_CU: YIELD" << endl;
#endif
          return YIELD;
        }
      sli->finished = true;
#ifdef TUA_DEBUG
      cout << "temporalUnitIntersection_CU_CU: CANCEL (3)" << endl;
#endif
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
          local.setAddr(Address(0));
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

#ifdef TUA_DEBUG
      cout << "temporalUnitIntersection_CU_C: received OPEN" << endl;
#endif
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

#ifdef TUA_DEBUG
      cout << "  uargindex =" << uargindex << endl;
#endif
      uv1 = (UT*) (u1.addr);
      uv2 = (T*) (u2.addr);

      if ( uv1->IsDefined() &&
           uv2->IsDefined() &&
           (uv1->constValue.Compare( uv2 ) == 0 ) )
        { // store result
          (sli->resultValues[sli->NoOfResults]).setAddr( uv1->Clone() );
          sli->NoOfResults++;
          sli->finished = false;
#ifdef TUA_DEBUG
          cout << "  Added Result" << endl;
#endif
        }// else: no result
      local.setAddr(sli);
#ifdef TUA_DEBUG
      cout << "temporalUnitIntersection_CU_C: finished OPEN" << endl;
#endif
      return 0;

    case REQUEST:

#ifdef TUA_DEBUG
      cout << "temporalUnitIntersection_CU_C: received REQUEST" << endl;
#endif
      if(local.addr == 0)
        {
#ifdef TUA_DEBUG
          cout << "temporalUnitIntersection_CU_C: finished REQUEST: "
               << "CANCEL (1)" << endl;
#endif
          return CANCEL;
        }
      sli = (TUIntersectionLocalInfo*) local.addr;
      if(sli->finished)
        {
#ifdef TUA_DEBUG
          cout << "temporalUnitIntersection_CU_C: finished REQUEST: "
               << "CANCEL (2)" << endl;
#endif
          return CANCEL;
        }
      if(sli->NoOfResultsDelivered < sli->NoOfResults)
        {
          result.setAddr( ((UT*)
            (sli->resultValues[sli->NoOfResultsDelivered].addr))->Clone() );
          ((UT*)(sli->resultValues[sli->NoOfResultsDelivered].addr))
            ->DeleteIfAllowed();
          sli->NoOfResultsDelivered++;
#ifdef TUA_DEBUG
          cout << "temporalUnitIntersection_CU_C: finished REQUEST: "
               << "YIELD" << endl;
#endif
          return YIELD;
        }
      sli->finished = true;
#ifdef TUA_DEBUG
      cout << "temporalUnitIntersection_CU_C: finished REQUEST: "
           << "CANCEL (3)" << endl;
#endif
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
          local.setAddr(Address(0));
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
struct MappingUnitIntersection_rLocalInfo {
  bool finished;
  int  NoOfResults;          // total number of results
  int  ResultsDelivered;     // number of results already delivered
  vector<UReal> resvector;   // the results
};

int temporalUnitIntersection_ureal_ureal( Word* args, Word& result, int message,
                                          Word& local, Supplier s )
{
  MappingUnitIntersection_rLocalInfo *localinfo;
  UReal *ureal1, *ureal2;
  Word a0, a1;
  Periods result_times(2);

  switch (message)
    {
    case OPEN :

      localinfo = new MappingUnitIntersection_rLocalInfo;
      local.setAddr(localinfo);
      localinfo->finished = true;
      localinfo->resvector.clear();
      localinfo->NoOfResults = 0;
      localinfo->ResultsDelivered = 0;

      // initialize arguments, such that a0 always contains the UReal
      //                       and a1 the CcReal
      a0 = args[0]; a1 = args[1];
      ureal1 = (UReal*)(a0.addr);
      ureal2 = (UReal*)(a1.addr);
      if ( !ureal1->IsDefined() ||
           !ureal2->IsDefined() )
      { // some input is undefined -> return empty stream
        return 0;
      }

      // call int UReal::PeriodsAtEqual( UReal& other, Periods& times)
      localinfo->NoOfResults = ureal1->PeriodsAtEqual( *ureal2, result_times);
//       cout << "temporalUnitIntersection_ureal_ureal(): NoOfResults="
//            << localinfo->NoOfResults << endl;
      localinfo->finished = (localinfo->NoOfResults <= 0);
      for(int i=0; i<localinfo->NoOfResults; i++)
      { // create result vector
        UReal unit(true);
        Interval<Instant> iv;
//      cout << "temporalUnitIntersection_ureal_ureal(): Processing interval "
//           << i << endl;
        result_times.Get(i, iv);
        if( iv.start == iv.end )
        { // simplify result to constant
          DateTime T(durationtype);
          T = iv.start - ureal1->timeInterval.start;
          double t = T.ToDouble();
          double value = ureal1->a*t*t + ureal1->b*t + ureal1->c;
          value = ureal1->r ? sqrt(value) : value;
          unit = UReal(iv, 0.0, 0.0, value, false);
        }
        else
          ureal1->AtInterval(iv, unit);
        localinfo->resvector.push_back(unit);
//         cout << "temporalUnitIntersection_ureal_ureal():  Added unit ";
//         unit.Print(cout);
        cout << endl;
      }
      localinfo->finished = ( localinfo->NoOfResults <= 0 );
//       cout << "temporalUnitIntersection_ureal_ureal(): NoOfResults="
//            << localinfo->NoOfResults << endl
//            << "temporalUnitIntersection_ureal_ureal(): finished="
//            << localinfo->finished << endl;
      return 0;

    case REQUEST :

      if (local.addr == 0)
        return CANCEL;
      localinfo = (MappingUnitIntersection_rLocalInfo*) local.addr;

      if (localinfo->finished)
        return CANCEL;
      if ( localinfo->NoOfResults <= localinfo->ResultsDelivered )
      { localinfo->finished = true;
        return CANCEL;
      }
      result =
          SetWord(localinfo->resvector[localinfo->ResultsDelivered].Clone());
      localinfo->ResultsDelivered++;
      return YIELD;

    case CLOSE :

      if (local.addr != 0)
      {
        localinfo = (MappingUnitIntersection_rLocalInfo*) local.addr;
        delete localinfo;
        local.setAddr(Address(0));
      }
      return 0;
    } // end switch

      // should not be reached
  return 0;
}



// value mapping for constant units (uT  T) -> (stream uT)
//                            and   ( T uT) -> (stream uT)
template<int uargindex>
int temporalUnitIntersection_ureal_real( Word* args, Word& result, int message,
                                         Word& local, Supplier s )
{
  MappingUnitIntersection_rLocalInfo *localinfo;
  UReal *uinput;
  CcReal *value;
  Word a0, a1;

  switch (message)
    {
    case OPEN :

      localinfo = new MappingUnitIntersection_rLocalInfo;
      local.setAddr(localinfo);
      localinfo->finished = true;
      localinfo->NoOfResults = 0;
      localinfo->ResultsDelivered = 0;

      // initialize arguments, such that a0 always contains the UReal
      //                       and a1 the CcReal
      if (uargindex == 0)
        { a0 = args[0]; a1 = args[1]; }
      else
        { a0 = args[1]; a1 = args[0]; }

      uinput = (UReal*)(a0.addr);
      value = (CcReal*)(a1.addr);
      if ( !uinput->IsDefined() ||
           !value->IsDefined() )
      { // some input is undefined -> return empty stream
        localinfo->NoOfResults = 0;
        localinfo->finished = true;
        return 0;
      }

      // call UReal::AtValue(CcReal value, vector<UReal>& result)
      localinfo->NoOfResults = uinput->AtValue(*value, localinfo->resvector);
      localinfo->finished = (localinfo->NoOfResults <= 0);
      return 0;

    case REQUEST :

      if (local.addr == 0)
        return CANCEL;
      localinfo = (MappingUnitIntersection_rLocalInfo*) local.addr;

      if (localinfo->finished)
        return CANCEL;
      if ( localinfo->NoOfResults <= localinfo->ResultsDelivered )
      { localinfo->finished = true;
        return CANCEL;
      }
      result =
          SetWord(localinfo->resvector[localinfo->ResultsDelivered].Clone());
      localinfo->ResultsDelivered++;
      return YIELD;

    case CLOSE :

      if (local.addr != 0)
      {
        localinfo = (MappingUnitIntersection_rLocalInfo*) local.addr;
        delete localinfo;
        local.setAddr(Address(0));
      }
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
  UPoint *uv1, *uv2, *res;

  // test for overlapping intervals
  switch( message )
    {
    case OPEN:

#ifdef TUA_DEBUG
      cout << "temporalUnitIntersection_upoint_upoint: received OPEN" << endl;
#endif
      sli = new TUIntersectionLocalInfo;
      sli->finished = true;
      sli->NoOfResults = 0;
      sli->NoOfResultsDelivered = 0;
      local.setAddr(sli);
      u1 = args[0];
      u2 = args[1];
      uv1 = (UPoint*) (u1.addr);
      uv2 = (UPoint*) (u2.addr);
      res = new UPoint( false );
      uv1->Intersection(*uv2, *res);
      if ( res->IsDefined() && res->timeInterval.Inside(uv1->timeInterval) &&
                               res->timeInterval.Inside(uv2->timeInterval) )
      { // 2nd and 3rd condition guarantees, that results on "open borders"
        // are filtered out
        (sli->resultValues[sli->NoOfResults]).setAddr( res );
        sli->NoOfResults++;
        sli->finished = false;
      }
      else
        delete( res );
#ifdef TUA_DEBUG
      cout << "temporalUnitIntersection_upoint_upoint: finished OPEN (6)"
           << endl;
#endif
      return 0;

    case REQUEST:

#ifdef TUA_DEBUG
      cout << "temporalUnitIntersection_upoint_upoint: received REQUEST"
           << endl;
#endif
      if(local.addr == 0)
        {
#ifdef TUA_DEBUG
          cout << "temporalUnitIntersection_upoint_upoint: CANCEL (1)"
               << endl;
#endif
          return CANCEL;
        }
      sli = (TUIntersectionLocalInfo*) local.addr;
      if(sli->finished)
        {
#ifdef TUA_DEBUG
          cout << "temporalUnitIntersection_upoint_upoint: CANCEL (2)"
               << endl;
#endif
          return CANCEL;
        }
#ifdef TUA_DEBUG
        cout << "  NoOfResults=" << sli->NoOfResults << endl
             << "  NoOfResultsDelivered=" << sli->NoOfResultsDelivered
             << endl;
#endif
      if(sli->NoOfResultsDelivered < sli->NoOfResults)
        {
          result.setAddr( ((UPoint*)
             (sli->resultValues[sli->NoOfResultsDelivered].addr))->Clone() );
          ((UPoint*)(sli->resultValues[sli->NoOfResultsDelivered].addr))
            ->DeleteIfAllowed();
          sli->NoOfResultsDelivered++;
#ifdef TUA_DEBUG
          cout << "temporalUnitIntersection_upoint_upoint: YIELD"
               << endl;
#endif
          return YIELD;
        }
      sli->finished = true;
#ifdef TUA_DEBUG
      cout << "temporalUnitIntersection_upoint_upoint: CANCEL (3)" << endl;
#endif
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
          local.setAddr(0);
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
  UPoint *unit, pResult(true);
  Point *val;


  switch( message )
    {
    case OPEN:

      sli = new TUIntersectionLocalInfo;
      sli->finished = true;
      sli->NoOfResults = 0;
      sli->NoOfResultsDelivered = 0;
      local.setAddr(sli);

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
          (sli->resultValues[sli->NoOfResults]).setAddr( pResult.Clone() );
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
          result.setAddr( ((UPoint*)
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
          local.setAddr(0);
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

calculates the periods where the given UPoint lies
inside the given Line. It returns the existing intervals in a Periods-Object.

*/
static void TUUPointInsideLine(UPoint *u, Line& ln, Periods& pResult)
{
#ifdef TUA_DEBUG
  cout<<"MPointLineInside called"<<endl;
#endif
  pResult.Clear();
  if( !u->IsDefined() || !ln.IsDefined() ){
    pResult.SetDefined( false );
    return;
  }
  pResult.SetDefined( true );

  HalfSegment l;

  const UPoint* up = (UPoint*) u;
  Periods* period = new Periods(0);
  Periods* between = new Periods(0);
  Point pt;
  Interval<Instant> newper; //part of the result

#ifdef TUA_DEBUG
  cout << "up = " << *up << endl;
#endif

  for( int n = 0; n < ln.Size(); n++)
  {
      Instant t;
      ln.Get(n, l);
#ifdef TUA_DEBUG
        cout << "up = " << *up << endl;
        cout << "ln: Segment # "<< n << " l = " << l << endl;
#endif
      if (l.GetRightPoint().GetX() == l.GetDomPoint().GetX()
       && l.GetRightPoint().GetY() == l.GetDomPoint().GetY()) {
#ifdef TUA_DEBUG
        cout<<"right point is dominating -> continue"<<endl;
#endif
        continue;
      }
      if(( l.GetRightPoint().GetX() < up->p0.GetX()
       &&  l.GetRightPoint().GetX() < up->p1.GetX())
       || (l.GetLeftPoint().GetX() > up->p0.GetX()
       &&  l.GetLeftPoint().GetX() > up->p1.GetX())
       || (l.GetRightPoint().GetY() < up->p0.GetY()
       &&  l.GetRightPoint().GetY() < up->p1.GetY()
       && (l.GetLeftPoint().GetY() < up->p0.GetY()
       &&  l.GetLeftPoint().GetY() < up->p1.GetY()))
       || (l.GetRightPoint().GetY() > up->p0.GetY()
       &&  l.GetRightPoint().GetY() > up->p1.GetY()
       && (l.GetLeftPoint().GetY() > up->p0.GetY()
       &&  l.GetLeftPoint().GetY() > up->p1.GetY()))) {
#ifdef TUA_DEBUG
        cout<<"Bounding Boxes not crossing!"<<endl;
#endif
        continue;
      }
      double al=0.0, bl=0.0, aup=0.0, bup=0.0;
      bool vl, vup;
      vl = l.GetRightPoint().GetX() == l.GetLeftPoint().GetX();
      if(!vl){
        al = (l.GetRightPoint().GetY() - l.GetLeftPoint().GetY())
           / (l.GetRightPoint().GetX() - l.GetLeftPoint().GetX());
        bl =  l.GetLeftPoint().GetY() - l.GetLeftPoint().GetX() * al;
#ifdef TUA_DEBUG
        cout<<"al: "<<al<<" bl: "<<bl<<endl;
#endif
      }
#ifdef TUA_DEBUG
      else
        cout<<"l is vertical"<<endl;
#endif
      vup = up->p1.GetX() == up->p0.GetX();
      if(!vup){
        aup = (up->p1.GetY() - up->p0.GetY())
            / (up->p1.GetX() - up->p0.GetX());
        bup =  up->p0.GetY() - up->p0.GetX() * aup;
#ifdef TUA_DEBUG
        cout<<"aup: "<<aup<<" bup: "<<bup<<endl;
#endif
      }
#ifdef TUA_DEBUG
      else
          cout<<"up is vertical"<<endl;
#endif
      if(vl && vup){
#ifdef TUA_DEBUG
          cout<<"both elements are vertical!"<<endl;
#endif
        if(up->p1.GetX() != l.GetLeftPoint().GetX()){
#ifdef TUA_DEBUG
          cout<<"elements are vertical but not at same line"<<endl;
#endif
          continue;
        }
        else {
#ifdef TUA_DEBUG
          cout<<"elements on same line"<<endl;
#endif
          if(up->p1.GetY() < l.GetLeftPoint().GetY()
           && up->p0.GetY() < l.GetLeftPoint().GetY()){
#ifdef TUA_DEBUG
            cout<<"uPoint lower as linesegment"<<endl;
#endif
            continue;
          }
          else if(up->p1.GetY() > l.GetRightPoint().GetY()
           && up->p0.GetY() > l.GetRightPoint().GetY()){
#ifdef TUA_DEBUG
            cout<<"uPoint higher as linesegment"<<endl;
#endif
            continue;
          }
          else{
#ifdef TUA_DEBUG
            cout<<"uPoint and linesegment partequal"<<endl;
#endif
            if (up->p0.GetY() <= l.GetLeftPoint().GetY()
             && up->p1.GetY() >= l.GetLeftPoint().GetY()){
#ifdef TUA_DEBUG
              cout<<"uPoint starts below linesegemet"<<endl;
#endif
              t.ReadFrom((l.GetLeftPoint().GetY() - up->p0.GetY())
                     / (up->p1.GetY() - up->p0.GetY())
                     * (up->timeInterval.end.ToDouble()
                     -  up->timeInterval.start.ToDouble())
                     +  up->timeInterval.start.ToDouble());
              t.SetType(instanttype);
#ifdef TUA_DEBUG
              cout<<"t "<<t.ToString()<<endl;
#endif
              newper.start = t;
              newper.lc = (up->timeInterval.start == t)
                         ? up->timeInterval.lc : true;
            }
            if(up->p1.GetY() <= l.GetLeftPoint().GetY()
             && up->p0.GetY() >= l.GetLeftPoint().GetY()){
#ifdef TUA_DEBUG
              cout<<"uPoint ends below linesegemet"<<endl;
#endif
              t.ReadFrom((l.GetLeftPoint().GetY() - up->p0.GetY())
                      / (up->p1.GetY() - up->p0.GetY())
                      * (up->timeInterval.end.ToDouble()
                      -  up->timeInterval.start.ToDouble())
                      +  up->timeInterval.start.ToDouble());
              t.SetType(instanttype);
#ifdef TUA_DEBUG
              cout<<"t "<<t.ToString()<<endl;
#endif
              newper.end = t;
              newper.rc = (up->timeInterval.end == t)
                         ? up->timeInterval.rc : true;
            }
            if(up->p0.GetY() <= l.GetRightPoint().GetY()
             && up->p1.GetY() >= l.GetRightPoint().GetY()){
#ifdef TUA_DEBUG
              cout<<"uPoint ends above linesegemet"<<endl;
#endif
              t.ReadFrom((l.GetRightPoint().GetY() - up->p0.GetY())
                      / (up->p1.GetY() - up->p0.GetY())
                      * (up->timeInterval.end.ToDouble()
                      -  up->timeInterval.start.ToDouble())
                      +  up->timeInterval.start.ToDouble());
              t.SetType(instanttype);
#ifdef TUA_DEBUG
              cout<<"t "<<t.ToString()<<endl;
#endif
              newper.end = t;
              newper.rc = (up->timeInterval.end == t)
                         ? up->timeInterval.rc : true;
            }
            if(up->p1.GetY() <= l.GetRightPoint().GetY()
             && up->p0.GetY() >= l.GetRightPoint().GetY()){
#ifdef TUA_DEBUG
              cout<<"uPoint starts above linesegemet"<<endl;
#endif
              t.ReadFrom((l.GetRightPoint().GetY() - up->p0.GetY())
                      / (up->p1.GetY() - up->p0.GetY())
                      * (up->timeInterval.end.ToDouble()
                      - up->timeInterval.start.ToDouble())
                      + up->timeInterval.start.ToDouble());
              t.SetType(instanttype);
#ifdef TUA_DEBUG
              cout<<"t "<<t.ToString()<<endl;
#endif
              newper.start = t;
              newper.lc = (up->timeInterval.start == t)
                         ? up->timeInterval.lc : true;
            }
            if (up->p0.GetY() <= l.GetRightPoint().GetY()
             && up->p0.GetY() >= l.GetLeftPoint().GetY()){
#ifdef TUA_DEBUG
              cout<<"uPoint starts inside linesegemet"<<endl;
#endif
              newper.start = up->timeInterval.start;
              newper.lc =    up->timeInterval.lc;
            }
            if( up->p1.GetY() <= l.GetRightPoint().GetY()
             && up->p1.GetY() >= l.GetLeftPoint().GetY()){
#ifdef TUA_DEBUG
              cout<<"uPoint ends inside linesegemet"<<endl;
#endif
              newper.end = up->timeInterval.end;
              newper.rc =  up->timeInterval.rc;
            }
            if(newper.start == newper.end
             && (!newper.lc || !newper.rc)){
#ifdef TUA_DEBUG
              cout<<"not an interval"<<endl;
#endif
              continue;
            }
          }
        }
      }
      else if(vl){
#ifdef TUA_DEBUG
        cout<<"vl is vertical vup not"<<endl;
#endif
        t.ReadFrom((l.GetRightPoint().GetX() - up->p0.GetX())
                / (up->p1.GetX() - up->p0.GetX())
                * (up->timeInterval.end.ToDouble()
                -  up->timeInterval.start.ToDouble())
                +  up->timeInterval.start.ToDouble());
        t.SetType(instanttype);
#ifdef TUA_DEBUG
        cout<<"t "<<t.ToString()<<endl;
#endif
        if((up->timeInterval.start == t && !up->timeInterval.lc)
         ||  (up->timeInterval.end == t && !up->timeInterval.rc))
          continue;

        if(up->timeInterval.start > t|| up->timeInterval.end < t){
#ifdef TUA_DEBUG
          cout<<"up outside line"<<endl;
#endif
          continue;
        }
        up->TemporalFunction(t, pt);
        if(  pt.GetX() < l.GetLeftPoint().GetX() ||
             pt.GetX() > l.GetRightPoint().GetX()
         || (pt.GetY() < l.GetLeftPoint().GetY() &&
             pt.GetY() < l.GetRightPoint().GetY())
         || (pt.GetY() > l.GetLeftPoint().GetY() &&
             pt.GetY() > l.GetRightPoint().GetY())){
#ifdef TUA_DEBUG
          cout<<"pt outside up!"<<endl;
#endif
          continue;
        }

        newper.start = t;
        newper.lc = true;
        newper.end = t;
        newper.rc = true;
      }
      else if(vup){
#ifdef TUA_DEBUG
        cout<<"vup is vertical vl not"<<endl;
#endif
        if(up->p1.GetY() != up->p0.GetY()) {
          t.ReadFrom((up->p0.GetX() * al + bl - up->p0.GetY())
                  / (up->p1.GetY() - up->p0.GetY())
                  * (up->timeInterval.end.ToDouble()
                  -  up->timeInterval.start.ToDouble())
                  +  up->timeInterval.start.ToDouble());
          t.SetType(instanttype);
#ifdef TUA_DEBUG
          cout<<"t "<<t.ToString()<<endl;
#endif
          if((up->timeInterval.start == t && !up->timeInterval.lc)
           ||  (up->timeInterval.end == t && !up->timeInterval.rc)){
#ifdef TUA_DEBUG
            cout<<"continue"<<endl;
#endif
            continue;
          }

          if(up->timeInterval.start > t|| up->timeInterval.end < t){
#ifdef TUA_DEBUG
            cout<<"up outside line"<<endl;
#endif
            continue;
          }
          up->TemporalFunction(t, pt);
          if(  pt.GetX() < l.GetLeftPoint().GetX() ||
               pt.GetX() > l.GetRightPoint().GetX()
           || (pt.GetY() < l.GetLeftPoint().GetY() &&
               pt.GetY() < l.GetRightPoint().GetY())
           || (pt.GetY() > l.GetLeftPoint().GetY() &&
               pt.GetY() > l.GetRightPoint().GetY())){
#ifdef TUA_DEBUG
            cout<<"pt outside up!"<<endl;
#endif
            continue;
          }

          newper.start = t;
          newper.lc = true;
          newper.end = t;
          newper.rc = true;
        }
        else {
#ifdef TUA_DEBUG
          cout<<"up is not moving"<<endl;
#endif
          if(al * up->p1.GetX() + bl == up->p1.GetY()){
#ifdef TUA_DEBUG
            cout<<"Point lies on line"<<endl;
#endif
            newper = up->timeInterval;
          }
          else {
#ifdef TUA_DEBUG
            cout<<"continue 2"<<endl;
#endif
            continue;
          }
        }
      }
      else if(aup == al){
#ifdef TUA_DEBUG
        cout<<"both lines have same gradient"<<endl;
#endif
        if(bup != bl){
#ifdef TUA_DEBUG
          cout<<"colinear but not equal"<<endl;
#endif
          continue;
        }
         if(up->p0.GetX() <= l.GetLeftPoint().GetX()
         && up->p1.GetX() >= l.GetLeftPoint().GetX()){
#ifdef TUA_DEBUG
           cout<<"uPoint starts left of linesegment"<<endl;
#endif
           t.ReadFrom((l.GetLeftPoint().GetX() - up->p0.GetX())
                   / (up->p1.GetX() - up->p0.GetX())
                   * (up->timeInterval.end.ToDouble()
                   -  up->timeInterval.start.ToDouble())
                   +  up->timeInterval.start.ToDouble());
           t.SetType(instanttype);
#ifdef TUA_DEBUG
           cout<<"t "<<t.ToString()<<endl;
#endif
           newper.start = t;
           newper.lc = (up->timeInterval.start == t)
                      ? up->timeInterval.lc : true;
        }
        if(up->p1.GetX() <= l.GetLeftPoint().GetX()
        && up->p0.GetX() >= l.GetLeftPoint().GetX()){
#ifdef TUA_DEBUG
          cout<<"uPoint ends left of linesegment"<<endl;
#endif
           t.ReadFrom((l.GetLeftPoint().GetX() - up->p0.GetX())
                   / (up->p1.GetX() - up->p0.GetX())
                   * (up->timeInterval.end.ToDouble()
                   -  up->timeInterval.start.ToDouble())
                   +  up->timeInterval.start.ToDouble());
           t.SetType(instanttype);
#ifdef TUA_DEBUG
           cout<<"t "<<t.ToString()<<endl;
#endif
           newper.end = t;
           newper.rc = (up->timeInterval.end == t)
                      ? up->timeInterval.rc : true;
        }
        if(up->p0.GetX() <= l.GetRightPoint().GetX()
        && up->p1.GetX() >= l.GetRightPoint().GetX()){
#ifdef TUA_DEBUG
          cout<<"uPoint ends right of linesegment"<<endl;
#endif
           t.ReadFrom((l.GetRightPoint().GetX() - up->p0.GetX())
                   / (up->p1.GetX() - up->p0.GetX())
                   * (up->timeInterval.end.ToDouble()
                   -  up->timeInterval.start.ToDouble())
                   +  up->timeInterval.start.ToDouble());
           t.SetType(instanttype);
#ifdef TUA_DEBUG
           cout<<"t "<<t.ToString()<<endl;
#endif
           newper.end = t;
           newper.rc = (up->timeInterval.end == t)
                      ? up->timeInterval.rc : true;
        }
        if(up->p1.GetX() <= l.GetRightPoint().GetX()
        && up->p0.GetX() >= l.GetRightPoint().GetX()){
#ifdef TUA_DEBUG
           cout<<"uPoint starts right of linesegment"<<endl;
#endif
           t.ReadFrom((l.GetRightPoint().GetX() - up->p0.GetX())
                   / (up->p1.GetX() - up->p0.GetX())
                   * (up->timeInterval.end.ToDouble()
                   -  up->timeInterval.start.ToDouble())
                   +  up->timeInterval.start.ToDouble());
           t.SetType(instanttype);
#ifdef TUA_DEBUG
           cout<<"t "<<t.ToString()<<endl;
#endif
           newper.start = t;
           newper.lc = (up->timeInterval.start == t)
                      ? up->timeInterval.lc : true;
        }
        if(up->p0.GetX() <= l.GetRightPoint().GetX()
        && up->p0.GetX() >= l.GetLeftPoint().GetX()){
#ifdef TUA_DEBUG
           cout<<"uPoint starts inside linesegment"<<endl;
#endif
           newper.start = up->timeInterval.start;
           newper.lc =    up->timeInterval.lc;
        }
        if(up->p1.GetX() <= l.GetRightPoint().GetX()
        && up->p1.GetX() >= l.GetLeftPoint().GetX()){
#ifdef TUA_DEBUG
          cout<<"uPoint ends inside linesegment"<<endl;
#endif
           newper.end = up->timeInterval.end;
           newper.rc =  up->timeInterval.rc;
        }
        if(newper.start == newper.end
        && (!newper.lc || !newper.rc)){
#ifdef TUA_DEBUG
          cout<<"not an interval"<<endl;
#endif
          continue;
        }
      } else{
#ifdef TUA_DEBUG
        cout<<"both lines have different gradients"<<endl;
#endif
        t.ReadFrom(((bl - bup) / (aup - al) - up->p0.GetX())
                / (up->p1.GetX() - up->p0.GetX())
                * (up->timeInterval.end.ToDouble()
                -  up->timeInterval.start.ToDouble())
                +  up->timeInterval.start.ToDouble());
        t.SetType(instanttype);
        if((up->timeInterval.start == t && !up->timeInterval.lc)
         ||  (up->timeInterval.end == t && !up->timeInterval.rc)){
#ifdef TUA_DEBUG
          cout<<"continue"<<endl;
#endif
          continue;
        }

        if(up->timeInterval.start > t|| up->timeInterval.end < t){
#ifdef TUA_DEBUG
          cout<<"up outside line"<<endl;
#endif
          continue;
        }
        up->TemporalFunction(t, pt);
        if(  pt.GetX() < l.GetLeftPoint().GetX() ||
             pt.GetX() > l.GetRightPoint().GetX()
         || (pt.GetY() < l.GetLeftPoint().GetY() &&
             pt.GetY() < l.GetRightPoint().GetY())
         || (pt.GetY() > l.GetLeftPoint().GetY() &&
             pt.GetY() > l.GetRightPoint().GetY())){
#ifdef TUA_DEBUG
          cout<<"pt outside up!"<<endl;
#endif
          continue;
        }

        newper.start = t;
        newper.lc = true;
        newper.end = t;
        newper.rc = true;
      }
#ifdef TUA_DEBUG
      cout<<"newper ["<< newper.start.ToString()<<" "<<newper.end.ToString()
          <<" "<<newper.lc<<" "<<newper.rc<<"]"<<endl;
#endif
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
      else{
        pResult.CopyFrom(period);
      }
  } // end for each segment
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
#ifdef TUA_DEBUG
    cout<<"TUCompletePeriods2MPoint called"<<endl;
#endif
  endResult->Clear();
  if( !u->IsDefined() || !pResult->IsDefined() ){
    endResult->SetDefined( false );
    return;
  }
  endResult->SetDefined( true );

  const UPoint* up = (UPoint*) u;
  Interval<Instant> per;
  UPoint newUp(true);
  Point pt;
  int m = 0;
  bool pfinished = (pResult->GetNoComponents() == 0);
  endResult->StartBulkLoad();
  for ( int i = 0; i < 1; i++) {
    if(!up->IsDefined())
        continue;
#ifdef TUA_DEBUG
    cout<<"*up = "<< *up <<endl;
#endif
    if(!pfinished) {
      pResult->Get(m, per);
#ifdef TUA_DEBUG
      cout<<"per "<<m<<" ["<<per.start.ToString()<<" "
        <<per.end.ToString()<<" "<<per.lc<<" "<<per.rc<<"]"<<endl;
#endif
    }
    if(pfinished) {
#ifdef TUA_DEBUG
      cout<<"no per any more. break 1"<<endl;
#endif
      break;
    }
    if(!(pfinished || up->timeInterval.end < per.start
     || (up->timeInterval.end == per.start
     && !up->timeInterval.rc && per.lc))) {
#ifdef TUA_DEBUG
      cout<<"per not totally after up"<<endl;
#endif
      if(up->timeInterval.start < per.start
       || (up->timeInterval.start == per.start
       && up->timeInterval.lc && !per.lc)) {
#ifdef TUA_DEBUG
        cout<<"up starts before per"<<endl;
#endif
        newUp.timeInterval = per;
      }
      else {
#ifdef TUA_DEBUG
        cout<<"per starts before or with up"<<endl;
#endif
        newUp.timeInterval.start = up->timeInterval.start;
        newUp.timeInterval.lc = up->timeInterval.lc;
      }
      while(true) {
        if(up->timeInterval.end < per.end
         || (up->timeInterval.end == per.end
         && per.rc && !up->timeInterval.rc)) {
#ifdef TUA_DEBUG
            cout<<"per ends after up (break)"<<endl;
#endif
            newUp.timeInterval.end = up->timeInterval.end;
            newUp.timeInterval.rc = up->timeInterval.rc;
            up->TemporalFunction(newUp.timeInterval.start, pt, true);
            newUp.p0 = pt;
            up->TemporalFunction(newUp.timeInterval.end, pt, true);
            newUp.p1 = pt;
#ifdef TUA_DEBUG
            cout<<"Add3 ("<<newUp.p0.GetX()<<" "<<newUp.p0.GetY()
              <<")->("<<newUp.p1.GetX()<<" "<<newUp.p1.GetY()
              <<") ["<<newUp.timeInterval.start.ToString()<<" "
              <<newUp.timeInterval.end.ToString()<<" "
              <<newUp.timeInterval.lc<<" "<<newUp.timeInterval.rc<<"]"<<endl;
#endif
            endResult->Add(newUp);
            break;
        }
        else {
#ifdef TUA_DEBUG
          cout<<"per ends inside up"<<endl;
#endif
          newUp.timeInterval.end = per.end;
          newUp.timeInterval.rc = per.rc;
          up->TemporalFunction(newUp.timeInterval.start, pt, true);
          newUp.p0 = pt;
          up->TemporalFunction(newUp.timeInterval.end, pt, true);
          newUp.p1 = pt;
#ifdef TUA_DEBUG
          cout<<"Add4 ("<<newUp.p0.GetX()<<" "<<newUp.p0.GetY()
             <<")->("<<newUp.p1.GetX()<<" "<<newUp.p1.GetY()
            <<") ["<<newUp.timeInterval.start.ToString()<<" "
            <<newUp.timeInterval.end.ToString()<<" "
            <<newUp.timeInterval.lc<<" "<<newUp.timeInterval.rc<<"]"<<endl;
#endif
          endResult->Add(newUp);
        }
        if(m == pResult->GetNoComponents() - 1){
#ifdef TUA_DEBUG
          cout<<"last per"<<endl;
#endif
          pfinished = true;
        }
        else {
          pResult->Get(++m, per);
#ifdef TUA_DEBUG
          cout<<"per "<<m<<" ["<<per.start.ToString()
            <<" "<<per.end.ToString()<<" "<<per.lc<<" "<<per.rc<<"]"<<endl;
#endif
        }
        if(!pfinished && (per.start < up->timeInterval.end
           || (per.start == up->timeInterval.end
           && up->timeInterval.rc && per.rc))){
#ifdef TUA_DEBUG
          cout<<"next per starts in same up"<<endl;
#endif
          newUp.timeInterval.start = per.start;
          newUp.timeInterval.lc = per.lc;
        }
        else {
#ifdef TUA_DEBUG
          cout<<"next interval after up -> finish up"<<endl;
#endif
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
  UPoint   res(true);
  UPoint  *u;
  Line    *l;
  Periods *p;

  UPoint cu;

  switch( message )
    {
    case OPEN:

#ifdef TUA_DEBUG
      cout << "temporalUnitIntersection_upoint_line<"
           << uargindex << ">: Received OPEN" << endl;
#endif
      p = new Periods(10);
      sli = new TUIntersectionLocalInfo;
      sli->finished = true;
      sli->NoOfResults = 0;
      sli->NoOfResultsDelivered = 0;
      sli->mpoint = new MPoint(10);
      local.setAddr(sli);

      // initialize arguments, such that a0 always contains the upoint
      //                       and a1 the line
#ifdef TUA_DEBUG
      cout << "  uargindex=" << uargindex << endl;
#endif
      if (uargindex == 0)
        { a0 = args[0]; a1 = args[1]; }
      else
        { a0 = args[1]; a1 = args[0]; }
      u = (UPoint*)(a0.addr);
      l = (Line*)(a1.addr);

      // test for definedness
      if ( !u->IsDefined() || !l->IsDefined() || l->IsEmpty() )
        {
#ifdef TUA_DEBUG
          cout << "  Undef/Empty arg -> Empty Result" << endl << endl;
#endif
          // nothing to do
        }
      else
        {
          TUUPointInsideLine(u, *l, *p);    // get intersecting timeintervals
          TUCompletePeriods2MPoint(u, p, sli->mpoint); // create upoints
          sli->NoOfResults = sli->mpoint->GetNoComponents();
          sli->finished = (sli->NoOfResults <= 0);
#ifdef TUA_DEBUG
          cout << "  " << sli->NoOfResults << " result units" << endl << endl;
#endif
        }
      delete p;
#ifdef TUA_DEBUG
      cout << "temporalUnitIntersection_upoint_line: Finished OPEN"
           << endl;
#endif
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
          result.setAddr( cu.Clone() );
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
          local.setAddr(0);
        }
      return 0;
    } // end switch

  return 0;
}

// for (upoint uregion) -> (stream upoint) (with <0, true>)
//     (uregion upoint) -> (stream upoint) (with <1, true>)
//     (upoint  region) -> (stream upoint) (with <0, false>)
//     (region  upoint) -> (stream upoint) (with <1, false>)
//
template<int uargindex, bool regionismoving>
int temporalUnitIntersection_upoint_uregion( Word* args, Word& result,
                                             int message,
                                             Word& local, Supplier s )
{
// This implementation uses class function
//   MRegion::Intersection(MPoint& mp, MPoint& res)
//   by creating a single-unit MRegion and a single-unit MPoint
// This is not very clever, but is comparable to the implementation found in
//   the MovingRegionAlgebra.

  TUIntersectionLocalInfo *sli;
  Word    a0, a1;
  UPoint  *u = 0;
  URegion *r = 0;
  Region  *f = 0;
  MPoint  *mp_tmp;
  MRegion *mr_tmp;
  UPoint cu;

  switch( message )
    {
    case OPEN:

#ifdef TUA_DEBUG
        cerr << "temporalUnitIntersection_upoint_uregion<"
             << uargindex << ", " << regionismoving
             << ">: Received OPEN" << endl;
#endif
      sli = new TUIntersectionLocalInfo;
      sli->finished = true;
      sli->NoOfResults = 0;
      sli->NoOfResultsDelivered = 0;
      sli->mpoint = new MPoint(10);
      local.setAddr(sli);

      // initialize arguments, such that a0 always contains the upoint
      //                       and a1 the uregion/region
#ifdef TUA_DEBUG
      cerr << "  uargindex=" << uargindex << endl;
#endif
      if (uargindex == 0)
        { a0 = args[0]; a1 = args[1]; }
      else if (uargindex == 1)
        { a0 = args[1]; a1 = args[0]; }
      else
        {
          cerr << "temporalUnitIntersection_upoint_uregion<"
               << uargindex << ", " << regionismoving
               << ">: WRONG uargindex!" << endl;
          return -1;
        }
      u = (UPoint*)(a0.addr);
      if ( regionismoving )
        r = (URegion*)(a1.addr);
      else
        f = (Region*) (a1.addr);

      // test for definedness
      if ( !u->IsDefined() ||
           ( regionismoving && !r->IsDefined()) ||
           (!regionismoving && !f->IsDefined()) )
        {
#ifdef TUA_DEBUG
            cerr << "  Undef arg -> Empty Result" << endl << endl;
#endif
          // nothing to do
        }
      else
        {
          mp_tmp = new MPoint(1); // create temporary MPoint
          mp_tmp->SetDefined(true);
          mp_tmp->Add(*u);

          // create temporary MRegion
          if ( regionismoving )
          { // case (upoint x uregion): from a URegion
            mr_tmp = new MRegion(1);
            mr_tmp->SetDefined(true);
            mr_tmp->AddURegion(*r);
            //mr_tmp.EndBulkLoad();
            assert( mr_tmp->IsDefined() );
          }
          else
          { // case (upoint x region): from (MPoint,Region)
             mr_tmp = new MRegion(*mp_tmp, *f);
             assert( mr_tmp->IsDefined() );
          }
          mr_tmp->Intersection(*mp_tmp, *(sli->mpoint)); // get and save result;
          delete mp_tmp;
          delete mr_tmp;
          sli->NoOfResults = sli->mpoint->GetNoComponents();
          sli->finished = (sli->NoOfResults <= 0);
#ifdef TUA_DEBUG
          cerr << "  " << sli->NoOfResults << " result units" << endl << endl;
#endif
        }
#ifdef TUA_DEBUG
      cerr << "temporalUnitIntersection_upoint_uregion: Finished OPEN"
           << endl;
#endif
      return 0;

    case REQUEST:
#ifdef TUA_DEBUG
        cerr << "temporalUnitIntersection_upoint_uregion<"
             << uargindex << ", " << regionismoving
             << ">: Received REQUEST" << endl;
#endif
      if(local.addr == 0)
        {
#ifdef TUA_DEBUG
          cerr << "temporalUnitIntersection_upoint_uregion<"
               << uargindex << ", " << regionismoving
               << ">: Finished REQUEST (1)" << endl;
#endif
          return CANCEL;
        }
      sli = (TUIntersectionLocalInfo*) local.addr;
      if(sli->finished)
        {
#ifdef TUA_DEBUG
          cerr << "temporalUnitIntersection_upoint_uregion<"
               << uargindex << ", " << regionismoving
               << ">: Finished REQUEST (2)" << endl;
#endif
          return CANCEL;
        }
      if(sli->NoOfResultsDelivered < sli->NoOfResults)
        {
          sli->mpoint->Get(sli->NoOfResultsDelivered, cu);
          result.setAddr( cu.Clone() );
          sli->NoOfResultsDelivered++;
#ifdef TUA_DEBUG
          cerr << "temporalUnitIntersection_upoint_uregion<"
               << uargindex << ", " << regionismoving
               << ">: Finished REQUEST (YIELD)" << endl;
#endif
          return YIELD;
        }
      sli->finished = true;
#ifdef TUA_DEBUG
      cerr << "temporalUnitIntersection_upoint_uregion<"
           << uargindex << ", " << regionismoving
           << ">: Finished REQUEST (3)" << endl;
#endif
      return CANCEL;

    case CLOSE:

#ifdef TUA_DEBUG
      cerr << "temporalUnitIntersection_upoint_uregion<"
           << uargindex << ", " << regionismoving
           << ">: Received CLOSE" << endl;
#endif
      if (local.addr != 0)
        {
          sli = (TUIntersectionLocalInfo*) local.addr;
          delete sli->mpoint;
          delete sli;
          local.setAddr(0);
        }
#ifdef TUA_DEBUG
      cerr << "temporalUnitIntersection_upoint_uregion<"
           << uargindex << ", " << regionismoving
           << ">: Finished CLOSE" << endl;
#endif
      return 0;
    } // end switch

  cerr << "temporalUnitIntersection_upoint_uregion<"
       << uargindex << ", " << regionismoving
       << ">: Received UNKNOWN COMMAND" << endl;
  return 0;
}

// For signatures
//     (upoint  region) -> (stream upoint) (with <0>)
//     (region  upoint) -> (stream upoint) (with <1>)

struct TUIntersectionUPointRegionLocalInfo
{
  TUIntersectionUPointRegionLocalInfo() :
      finished(true), NoOfResults(0), NoOfResultsDelivered(0)
  { results.clear(); }

  ~TUIntersectionUPointRegionLocalInfo() {}

  bool finished;
  int  NoOfResults;
  int  NoOfResultsDelivered;
  vector<UPoint> results;
};

template<int uargindex>
int temporalUnitIntersection_upoint_region( Word* args, Word& result,
                                            int message,
                                            Word& local, Supplier s )
{
  TUIntersectionUPointRegionLocalInfo *sli =
    static_cast<TUIntersectionUPointRegionLocalInfo*>(local.addr);
  Word    a0, a1;
  UPoint  *u = 0;
  Region  *r = 0;
  UPoint cu;

  switch( message )
    {
    case OPEN:

      if(sli){
        delete sli;
      }
      sli = new TUIntersectionUPointRegionLocalInfo;
      local.setAddr(sli);

      // initialize arguments, such that a0 always contains the upoint
      //                       and a1 the uregion/region
      if (uargindex == 0) {
        a0 = args[0]; a1 = args[1];
      } else if (uargindex == 1) {
        a0 = args[1]; a1 = args[0];
      } else {
        cerr << __PRETTY_FUNCTION__ << ": WRONG uargindex!" << endl;
        return -1;
      }
      u = static_cast<UPoint*>(a0.addr);
      r = static_cast<Region*>(a1.addr);
      // test for definedness
      if ( u->IsDefined() && r->IsDefined() ){
        if(u->AtRegion(r, sli->results)) {
          sli->NoOfResults = sli->results.size();
          sli->finished = (sli->NoOfResults <= 0);
        } else {
          cerr << __PRETTY_FUNCTION__ << ": INFO: UPoint::AtRegion failed!"
               << endl;
        }
      }
      return 0;

    case REQUEST:

      if(    !sli
          || sli->finished
          || (sli->NoOfResultsDelivered >= sli->NoOfResults)
        ){
        sli->finished = true;
        return CANCEL;
      } else {
        result.setAddr(sli->results[sli->NoOfResultsDelivered].Clone());
        sli->NoOfResultsDelivered++;
        return YIELD;
      }

    case CLOSE:

      if (sli) {
        delete sli;
        local.setAddr(0);
      }
      return 0;
    } // end switch

  cerr << __PRETTY_FUNCTION__ << ": Received UNKNOWN COMMAND" << endl;
  return 0;
}

template<int uargindex>
int temporalUnitIntersection_uregion_region( Word* args, Word& result,
                                             int message,
                                             Word& local, Supplier s )
{
  cerr << "temporalUnitIntersection_uregion_region(): Not yet Implemented!"
       << endl;
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
  "(uT uT) -> (stream uT)*\n"
  "(uT  T) -> (stream uT)\n"
  "( T uT) -> (stream uT)\n"
  "(line upoint) -> (stream upoint)\n"
  "(upoint line) -> (stream upoint)\n"
  "(upoint uregion) -> (stream upoint)\n"
  "(uregion upoint) -> (stream upoint)\n"
  "(upoint region) -> (stream upoint)\n"
  "(region upoint) -> (stream upoint)\n"
  "(*): Not yet implemented for T = real</text--->"
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
    temporalUnitIntersection_CU_CU<UBool>,           // 0
    temporalUnitIntersection_CU_CU<UInt>,
    temporalUnitIntersection_ureal_ureal,
    temporalUnitIntersection_upoint_upoint,
    temporalUnitIntersection_CU_CU<UString>,

    temporalUnitIntersection_CU_C<UBool, CcBool, 0>, // 5
    temporalUnitIntersection_CU_C<UInt, CcInt, 0>,
    temporalUnitIntersection_ureal_real<0>,
    temporalUnitIntersection_upoint_point<0>,
    temporalUnitIntersection_CU_C<UString, CcString, 0>,

    temporalUnitIntersection_CU_C<UBool, CcBool, 1>, // 10
    temporalUnitIntersection_CU_C<UInt, CcInt, 1>,
    temporalUnitIntersection_ureal_real<1>,
    temporalUnitIntersection_upoint_point<1>,
    temporalUnitIntersection_CU_C<UString, CcString, 1>,

    temporalUnitIntersection_upoint_line<0>,         // 15
    temporalUnitIntersection_upoint_line<1>,

    temporalUnitIntersection_upoint_uregion<0, true>,
    temporalUnitIntersection_upoint_uregion<1, true>,

    temporalUnitIntersection_upoint_region<0>,
    temporalUnitIntersection_upoint_region<1>        // 20
  };

int temporalunitIntersectionSelect( ListExpr args )
{
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  if( nl->IsEqual( arg1, UBool::BasicType() )   &&
      nl->IsEqual( arg2, UBool::BasicType()) )   return 0;
  if( nl->IsEqual( arg1, UInt::BasicType() )    &&
      nl->IsEqual( arg2, UInt::BasicType() ) )    return 1;
  if( nl->IsEqual( arg1, UReal::BasicType() )   &&
      nl->IsEqual( arg2, UReal::BasicType() ) )   return 2;
  if( nl->IsEqual( arg1, UPoint::BasicType() )  &&
      nl->IsEqual( arg2, UPoint::BasicType() ) )  return 3;
  if( nl->IsEqual( arg1, UString::BasicType() ) &&
      nl->IsEqual( arg2, UString::BasicType() ) ) return 4;

  if( nl->IsEqual( arg1, UBool::BasicType() )   &&
      nl->IsEqual( arg2, CcBool::BasicType() ))     return 5;
  if( nl->IsEqual( arg1, UInt::BasicType() )    &&
      nl->IsEqual( arg2, CcInt::BasicType() ))      return 6;
  if( nl->IsEqual( arg1, UReal::BasicType() )   &&
      nl->IsEqual( arg2, CcReal::BasicType() ))     return 7;
  if( nl->IsEqual( arg1, UPoint::BasicType() )  &&
      nl->IsEqual( arg2, Point::BasicType()  ) )  return 8;
  if( nl->IsEqual( arg1, UString::BasicType() ) &&
      nl->IsEqual( arg2, CcString::BasicType() ))   return 9;

  if( nl->IsEqual( arg2, UBool::BasicType() )   &&
      nl->IsEqual( arg1, CcBool::BasicType() ))     return 10;
  if( nl->IsEqual( arg2, UInt::BasicType() )    &&
      nl->IsEqual( arg1, CcInt::BasicType() ))      return 11;
  if( nl->IsEqual( arg2, UReal::BasicType() )   &&
      nl->IsEqual( arg1, CcReal::BasicType() ))     return 12;
  if( nl->IsEqual( arg2, UPoint::BasicType() )  &&
      nl->IsEqual( arg1, Point::BasicType() ))    return 13;
  if( nl->IsEqual( arg2, UString::BasicType() ) &&
      nl->IsEqual( arg1, UString::BasicType() ))  return 14;

  if( nl->IsEqual( arg1, UPoint::BasicType() )    &&
      nl->IsEqual( arg2, Line::BasicType() ))       return 15;
  if( nl->IsEqual( arg1, Line::BasicType() )      &&
      nl->IsEqual( arg2, UPoint::BasicType() ) )    return 16;
  if( nl->IsEqual( arg1, UPoint::BasicType() )    &&
      nl->IsEqual( arg2, URegion::BasicType() ) )   return 17;
  if( nl->IsEqual( arg1, URegion::BasicType() )   &&
      nl->IsEqual( arg2, UPoint::BasicType() ) )    return 18;
  if( nl->IsEqual( arg1, UPoint::BasicType() )    &&
      nl->IsEqual( arg2, Region::BasicType() ) )    return 19;
  if( nl->IsEqual( arg1, Region::BasicType() )    &&
      nl->IsEqual( arg2, UPoint::BasicType() ) )    return 20;

  cerr << "ERROR: Unmatched case in temporalunitIntersectionSelect" << endl;
  string argstr;
  nl->WriteToString(argstr, args);
  cerr << "       Argumets = '" << argstr << "'." << endl;
  return -1;
}


/*
5.26.5 Definition of operator ~intersection~

*/

Operator temporalunitintersection( "intersection",
                                   TemporalUnitIntersectionSpec,
                                   21,
                                   temporalunitintersectionmap,
                                   temporalunitIntersectionSelect,
                                   TemporalUnitIntersectionTypeMap);


/*
5.13 Operator ~at~

The operator restricts a unit type to the interval, where it's value
is equal to a given value. For base types ~bool~, ~int~ and ~point~,
the result will be only a single unit, but for base type ~real~, there
may be two units, as ~ureal~ is represented by a quadratic polynomial
function (or it's radical).

5.13.1 Type Mapping for ~at~

----
      at:     For T in {bool, int, string, point, region*}
OK  +                           uT x       T --> uT
OK                           ureal x    real --> (stream ureal)
OK  +                       upoint x  region --> (stream upoint)
                            upoint x  rect   --> upoint

(*): Not yet implemented

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

#ifdef TUA_DEBUG
    cout << "\nTemporalUnitAtTypeMapUnit: 0" << endl;
#endif
    if( nl->IsEqual( arg1, UBool::BasicType() )
      && nl->IsEqual( arg2, CcBool::BasicType() ) )
      return nl->TwoElemList(nl->SymbolAtom( Symbol::STREAM() ),
                             nl->SymbolAtom( UBool::BasicType() ));
    if( nl->IsEqual( arg1, UInt::BasicType() )
      && nl->IsEqual( arg2, CcInt::BasicType() ) )
      return nl->TwoElemList(nl->SymbolAtom( Symbol::STREAM() ),
                             nl->SymbolAtom( UInt::BasicType() ));
    if( nl->IsEqual( arg1, UPoint::BasicType() )
      && nl->IsEqual( arg2, Point::BasicType() ) )
      return nl->TwoElemList(nl->SymbolAtom( Symbol::STREAM() ),
                             nl->SymbolAtom( UPoint::BasicType() ));
    // for ureal, _ at _ will return a stream of ureals!
    if( nl->IsEqual( arg1, UReal::BasicType() )
      && nl->IsEqual( arg2, CcReal::BasicType() ) )
      return nl->TwoElemList(nl->SymbolAtom( Symbol::STREAM() ),
                             nl->SymbolAtom( UReal::BasicType() ));
    if( nl->IsEqual( arg1, UString::BasicType() )
      && nl->IsEqual( arg2, CcString::BasicType() ) )
      return nl->TwoElemList(nl->SymbolAtom( Symbol::STREAM() ),
                             nl->SymbolAtom( UString::BasicType() ));
    if( nl->IsEqual( arg1, URegion::BasicType() )
      && nl->IsEqual( arg2, Region::BasicType() ) )
      return nl->TwoElemList(nl->SymbolAtom( Symbol::STREAM() ),
                             nl->SymbolAtom( URegion::BasicType() ));
    if( nl->IsEqual( arg1, UPoint::BasicType() )
      && nl->IsEqual( arg2, Region::BasicType() ) )
      return nl->TwoElemList(nl->SymbolAtom( Symbol::STREAM() ),
                             nl->SymbolAtom( UPoint::BasicType() ));
    if( nl->IsEqual( arg1, UPoint::BasicType() )
      && nl->IsEqual( arg2, Rectangle<2>::BasicType() ) )
      return nl->SymbolAtom( UPoint::BasicType() );
  }
#ifdef TUA_DEBUG
  cout << "\nTemporalUnitAtTypeMapUnit: 1" << endl;
#endif
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

/*
5.13.2 Value Mapping for ~at~

Instead of implementing dedicated value mappings, we use those for operator
~intersection~ except for "upoint x rect".

*/


int AtUpR(Word* args, Word& result,
          int message, Word& local, Supplier s) {

  result = qp->ResultStorage(s);
  UPoint* arg1 = static_cast<UPoint*>(args[0].addr);
  Rectangle<2>* arg2 = static_cast<Rectangle<2>*>(args[1].addr);
  UPoint* res = static_cast<UPoint*>(result.addr);
  arg1->At(*arg2,*res);
  return 0;
}


/*
5.13.3 Specification for operator ~at~

*/
const string
TemporalSpecAt =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>For T in {bool, int, string, point, region*}:\n"
"(uT     T     ) -> (stream uT)\n"
"(ureal  real  ) -> (stream ureal)\n"
"(upoint region) -> (stream upoint)\n"
"(upoint rect) -> upoint\n"
"(*): Not yet implemented</text--->"
"<text>_ at _ </text--->"
"<text>restrict the movement to the times "
"where the equality occurs.\n"
"Observe, that the result is always a "
"'(stream UNIT)' rather than a 'UNIT'! except for upoint x rect</text--->"
"<text>upoint1 at point1 the_mvalue</text---> ) )";

/*
5.13.4 Selection Function of operator ~at~

*/
int
TUSelectAt( ListExpr args )
{
  ListExpr arg1 = nl->First( args ),
           arg2 = nl->Second( args );

  if( nl->SymbolValue( arg1 ) == UBool::BasicType() &&
      nl->SymbolValue( arg2 ) == CcBool::BasicType() )
    return 0;

  if( nl->SymbolValue( arg1 ) == UInt::BasicType() &&
      nl->SymbolValue( arg2 ) == CcInt::BasicType() )
    return 1;

  if( nl->SymbolValue( arg1 ) == UReal::BasicType() &&
      nl->SymbolValue( arg2 ) == CcReal::BasicType() )
    return 2;

  if( nl->SymbolValue( arg1 ) == UPoint::BasicType() &&
      nl->SymbolValue( arg2 ) == Point::BasicType() )
    return 3;

  if( nl->SymbolValue( arg1 ) == UString::BasicType() &&
      nl->SymbolValue( arg2 ) == CcString::BasicType() )
    return 4;

  if( nl->SymbolValue( arg1 ) == URegion::BasicType() &&
      nl->SymbolValue( arg2 ) == Region::BasicType() )
    return 5;

  if( nl->SymbolValue( arg1 ) == UPoint::BasicType() &&
      nl->SymbolValue( arg2 ) == Region::BasicType() )
    return 6;

  if( nl->SymbolValue( arg1 ) == UPoint::BasicType() &&
      nl->SymbolValue( arg2 ) == Rectangle<2>::BasicType() )
    return 7;

  return -1; // This point should never be reached
}


ValueMapping temporalunitatmap[] = {
  temporalUnitIntersection_CU_C< UBool, CcBool, 0 >,          //0
  temporalUnitIntersection_CU_C< UInt, CcInt, 0 >,            //1
  temporalUnitIntersection_ureal_real<0>,                     //2
  temporalUnitIntersection_upoint_point<0>,                   //3
  temporalUnitIntersection_CU_C< UString, CcString, 0 >,      //4
  temporalUnitIntersection_uregion_region<0>,                 //5
  temporalUnitIntersection_upoint_region<0>,                  //6
  AtUpR};                                                     //7

/*
5.13.5  Definition of operator ~at~

*/
Operator temporalunitat( "at",
                     TemporalSpecAt,
                     8,
                     temporalunitatmap,
                     TUSelectAt,
                     TemporalUnitAtTypeMapUnit );



/*
5.31 Operator ~no\_components~

Return the number of components (units) contained by the object. For unit types,
the result is either undef (for undef values) or a const unit with value=1
(otherwise).

----
     n/a + no_components:     (uT) --> uint

----

*/

/*
5.31.1 Type mapping function for ~no\_components~

*/

static ListExpr TUNoComponentsTypeMap(ListExpr args) {

  if (nl->ListLength(args) == 1)
    {
      if (nl->IsEqual(nl->First(args), UBool::BasicType()))
        return nl->SymbolAtom(UInt::BasicType());
      if (nl->IsEqual(nl->First(args), UInt::BasicType()))
        return nl->SymbolAtom(UInt::BasicType());
      if (nl->IsEqual(nl->First(args), UReal::BasicType()))
        return nl->SymbolAtom(UInt::BasicType());
      if (nl->IsEqual(nl->First(args), UString::BasicType()))
        return nl->SymbolAtom(UInt::BasicType());
      if (nl->IsEqual(nl->First(args), UPoint::BasicType()))
        return nl->SymbolAtom(UInt::BasicType());
      if (nl->IsEqual(nl->First(args), URegion::BasicType()))
        return nl->SymbolAtom(UInt::BasicType());
    }
  return nl->SymbolAtom(Symbol::TYPEERROR());
}

/*
5.31.2 Value mapping for operator ~no\_components~

*/

template<class T>
int TUNoComponentsValueMap(Word* args, Word& result,
                           int message, Word& local, Supplier s)
{
  result = (qp->ResultStorage( s ));
  UInt  *res   = (UInt*)result.addr;
  T     *input = (T*)args[0].addr;

  if ( input->IsDefined() ) {
      res->SetDefined(true);
      res->timeInterval.CopyFrom(input->timeInterval);
      res->constValue.Set(true,1);
  } else {
      res->SetDefined(false);
      res->constValue.Set(true,0);
  }
  return 0;
}

/*
5.31.3 Specification for operator ~no\_components~

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
5.31.4 Selection Function of operator ~no\_components~

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
5.31.5 Definition of operator ~no\_components~

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
  ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom(Symbol::ERRORS()));

  if ( ( nl->ListLength(args) == 1 ) && ( nl->IsAtom(nl->First(args) ) ) )
    {
      arg1 = nl->First(args);
      if( am->CheckKind(Kind::UNIT(), arg1, errorInfo) )
        return nl->SymbolAtom(CcBool::BasicType());
    }
  ErrorReporter::ReportError("Operator isempty expects a list of length one, "
                             "containing a value of type 'U' with U in "
                             "kind UNIT.");
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

/*
5.32.2 Value mapping for operator ~isempty~

*/
int TUIsemptyValueMap( Word* args, Word& result, int message,
                       Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  Attribute* val = ((Attribute*)args[0].addr);
  ((CcBool *)result.addr)->Set( true, !val->IsDefined() );
  return 0;
}

/*
5.32.3 Specification for operator ~isempty~

*/
const string TUIsemptySpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>For U in kind UNIT:\n"
  "U -> bool</text--->"
  "<text>isempty( _ )</text--->"
  "<text>The operator returns TRUE, if the unit is undefined, "
  "otherwise it returns FALSE. It will never return an "
  "undefined result!</text--->"
  "<text>query isempty([const uint value undef])</text--->"
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

    if( nl->IsEqual( arg1, UBool::BasicType() )  )
      return nl->SymbolAtom( UBool::BasicType() );
  }
  return nl->SymbolAtom( Symbol::TYPEERROR() );
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

  if(!input->IsDefined()){
    res->SetDefined( false );
  } else {
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
  if( nl->ListLength( args ) == 2 )
    {
      ListExpr arg1 = nl->First( args );
      ListExpr arg2 = nl->Second( args );

      // First case: ubool ubool -> ubool
      if ( nl->IsEqual( arg1, UBool::BasicType() )
        && nl->IsEqual( arg2, UBool::BasicType() ) )
        return nl->SymbolAtom(UBool::BasicType());

      // Second case: ubool bool -> ubool
      if( nl->IsEqual( arg1, UBool::BasicType() )
        && nl->IsEqual( arg2, CcBool::BasicType()) )
        return nl->SymbolAtom(UBool::BasicType());

      // Third case: bool ubool -> ubool
      if( nl->IsEqual( arg1, CcBool::BasicType() )
        && nl->IsEqual( arg2, UBool::BasicType()) )
        return nl->SymbolAtom(UBool::BasicType());

      // Error case:
      string argstr1, argstr2;
      nl->WriteToString(argstr1, arg1);
      nl->WriteToString(argstr2, arg2);
      ErrorReporter::ReportError(
        "Binary booleon operator expects any combination of {bool, ubool} "
        "as arguments, but the passed arguments have types '"+ argstr1 +
        "' and '" + argstr2 + "'.");
    } else {
      ErrorReporter::ReportError(
          "Binary booleon operator expects any combination of {bool, ubool} "
          "as arguments.");
    }
  return nl->SymbolAtom(Symbol::TYPEERROR());
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
      cb = (CcBool*)args[1].addr;
    }
  else if(ArgConf == 2) // case: bool x ubool -> ubool
    {
      u1 = (UBool*)args[1].addr;
      cb = (CcBool*)args[0].addr;
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
      cb = (CcBool*)args[1].addr;
    }
  else if(ArgConf == 2) // case: bool x ubool -> ubool
    {
      u1 = (UBool*)args[1].addr;
      cb = (CcBool*)args[0].addr;
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

  if( nl->SymbolValue( arg1 ) == UBool::BasicType()
   && nl->SymbolValue( arg2 ) == UBool::BasicType() )
    return 0;
  if( nl->SymbolValue( arg1 ) == UBool::BasicType()
   && nl->SymbolValue( arg2 ) == CcBool::BasicType() )
    return 1;
  if( nl->SymbolValue( arg1 ) == CcBool::BasicType()
   && nl->SymbolValue( arg2 ) == UBool::BasicType() )
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
 3,
 temporalunitAndValuemap,
 TUBinaryBoolFuncSelect,
 TUBinaryBoolFuncTypeMap
);

Operator temporalunitor
(
 "or",
 TUOrSpec,
 3,
 temporalunitOrValuemap,
 TUBinaryBoolFuncSelect,
 TUBinaryBoolFuncTypeMap
);


/*
5.35 Operator ~ComparePredicates~

Here, we implement the binary comparison operators/predicates for (uT uT).
The predicates are:

----

  == (equality),
  ## (unequality),
  << (smaller than),
  >> (bigger than),
  <<== (smaller than or equal to),
  >>== (bigger than or equal to).

----

The operators use the internal ~Compare~ function, which implements an 
ordering on the elements, but does not need to respect intuitive operator 
semantics (e.g. in case ureal).
They compare whole units as such. They do NOT compare the values!

WARNING: Do not confuse this operators with the ~CompareValuePredicates~, which
         compare the values.

----
      =, #, <, >, <=, >=: For T in {int, bool, real, string, point, region}
n/a +        uT x uT --> bool

----

*/

/*
5.35.1 Type mapping function for ~ComparePredicates~

*/
ListExpr TUComparePredicatesTypeMap( ListExpr args )
{
  if( nl->ListLength( args ) == 2 )
    {
      ListExpr arg1 = nl->First( args );
      ListExpr arg2 = nl->Second( args );
      if (nl->Equal( arg1, arg2 ))
        {
          if( (nl->IsEqual( arg1, UBool::BasicType() ) )   ||
              (nl->IsEqual( arg1, UInt::BasicType() ) )    ||
              (nl->IsEqual( arg1, UString::BasicType() ) ) ||
              (nl->IsEqual( arg1, UReal::BasicType() ) )   ||
              (nl->IsEqual( arg1, URegion::BasicType() ) ) ||
              (nl->IsEqual( arg1, UPoint::BasicType() ) ) )
            return nl->SymbolAtom( CcBool::BasicType() );
        }
        string argstr1, argstr2;
        nl->WriteToString(argstr1, arg1);
        nl->WriteToString(argstr2, arg2);
        ErrorReporter::ReportError(
            "Compare Operator (one of ==, ##, <<, >>, , <<==, >>==) expects "
            "two arguments of type 'uT', where T in {bool, int, real, "
            "string, point, region}./nThe passed arguments have types '"
            + argstr1 +"' and '" + argstr2 + "'.");
    } else {
    // Error case:
    ErrorReporter::ReportError(
      "Compare Operator (one of ==, ##, <<, >>, , <<==, >>==) expects "
      "two arguments of type 'uT', where T in {bool, int, real, "
      "string, point, region}./n");
    }
  return nl->SymbolAtom(Symbol::TYPEERROR());
}
/*
5.35.2 Value mapping for operator ~ComparePredicates~

template parameter ~OPType~ gives the character of the operator:

----

  0  =,
  1  #,
  2  <,
  3  >,
  4 <=,
  5 >=-

----

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
const string TUEqSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(uT uT) -> bool\n</text--->"
  "<text>_ == _</text--->"
  "<text>The operator checks if the internal ordering predicate "
  "holds for both arguments.</text--->"
  "<text>query [const ubool value ((\"2010-11-11\" "
  "\"2011-01-03\" TRUE FALSE) TRUE)] == [const ubool value "
  "((\"2011-01-01\" \"2012-09-17\" FALSE TRUE) TRUE)]</text--->"
  ") )";

const string TUNEqSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(uT uT) -> bool\n</text--->"
  "<text>_ ## _</text--->"
  "<text>The operator checks if the internal ordering predicate "
  "holds for both arguments.</text--->"
  "<text>query [const ubool value ((\"2010-11-11\" "
  "\"2011-01-03\" TRUE FALSE) TRUE)] ## [const ubool value "
  "((\"2011-01-01\" \"2012-09-17\" FALSE TRUE) TRUE)]</text--->"
  ") )";

const string TULtSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(uT uT) -> bool\n</text--->"
  "<text>_ << _</text--->"
  "<text>The operator checks if the internal ordering predicate "
  "holds for both arguments.</text--->"
  "<text>query [const ubool value ((\"2010-11-11\" "
  "\"2011-01-03\" TRUE FALSE) TRUE)] << [const ubool value "
  "((\"2011-01-01\" \"2012-09-17\" FALSE TRUE) TRUE)]</text--->"
  ") )";

const string TUBtSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(uT uT) -> bool\n</text--->"
  "<text>_ >> _</text--->"
  "<text>The operator checks if the internal ordering predicate "
  "holds for both arguments.</text--->"
  "<text>query [const ubool value ((\"2010-11-11\" "
  "\"2011-01-03\" TRUE FALSE) TRUE)] >> [const ubool value "
  "((\"2011-01-01\" \"2012-09-17\" FALSE TRUE) TRUE)]</text--->"
  ") )";

const string TULtEqSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(uT uT) -> bool\n</text--->"
  "<text>_ <<== _</text--->"
  "<text>The operator checks if the internal ordering predicate "
  "holds for both arguments.</text--->"
  "<text>query [const ubool value ((\"2010-11-11\" "
  "\"2011-01-03\" TRUE FALSE) TRUE)] <<== [const ubool value "
  "((\"2011-01-01\" \"2012-09-17\" FALSE TRUE) TRUE)]</text--->"
  ") )";

const string TUBtEqSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(uT uT) -> bool\n</text--->"
  "<text>_ >>== _</text--->"
  "<text>The operator checks if the internal ordering predicate "
  "holds for both arguments.</text--->"
  "<text>query [const ubool value ((\"2010-11-11\" "
  "\"2011-01-03\" TRUE FALSE) TRUE)] >>== [const ubool value "
  "((\"2011-01-01\" \"2012-09-17\" FALSE TRUE) TRUE)]</text--->"
  ") )";

/*
5.35.4 Selection Function of operator ~ComparePredicates~

We can use Operator::SimpleSelect:

*/

/*
5.35.5 Definition of operator ~ComparePredicates~

*/
Operator temporalunitisequal
(
 "==",
 TUEqSpec,
 TUComparePredicatedValueMap<0>,
 Operator::SimpleSelect,
 TUComparePredicatesTypeMap
 );

Operator temporalunitisnotequal
(
 "##",
 TUNEqSpec,
 TUComparePredicatedValueMap<1>,
 Operator::SimpleSelect,
 TUComparePredicatesTypeMap
 );

Operator temporalunitsmaller
(
 "<<",
 TULtSpec,
 TUComparePredicatedValueMap<2>,
 Operator::SimpleSelect,
 TUComparePredicatesTypeMap
 );

Operator temporalunitbigger
(
 ">>",
 TUBtSpec,
 TUComparePredicatedValueMap<3>,
 Operator::SimpleSelect,
 TUComparePredicatesTypeMap
 );

Operator temporalunitsmallereq
(
 "<<==",
 TULtEqSpec,
 TUComparePredicatedValueMap<4>,
 Operator::SimpleSelect,
 TUComparePredicatesTypeMap
 );

Operator temporalunitbiggereq
(
 ">>==",
 TUBtEqSpec,
 TUComparePredicatedValueMap<5>,
 Operator::SimpleSelect,
 TUComparePredicatesTypeMap
 );
/*
5.36 Operator ~uint2ureal~

This operator creates an ureal value from an uint value

----
     uint --> ureal

----

*/

/*
5.36.1 Type mapping function for ~uint2ureal~

*/
ListExpr TUuint2urealTypeMap( ListExpr args )
{
  if( nl->ListLength( args ) == 1 )
    {
      ListExpr arg1 = nl->First( args );
      if( nl->IsEqual( arg1, UInt::BasicType() ) )
        return nl->SymbolAtom(UReal::BasicType());
      // Error case:
      string argstr1;
      nl->WriteToString(argstr1, arg1);
      ErrorReporter::ReportError(
          "Operator uint2ureal expects an argument of type 'uint', "
          "but the passed argument has type '"+ argstr1 + "'.");
  } else {
    ErrorReporter::ReportError(
        "Operator uint2ureal expects an argument of type 'uint'.");
  }
  return nl->SymbolAtom(Symbol::TYPEERROR());
}
/*
5.36.2 Value mapping for operator ~uint2ureal~

*/
int TUuint2urealValueMap(Word* args, Word& result, int message,
                                Word& local, Supplier s)
{
  result = (qp->ResultStorage( s ));
  UReal *res = (UReal*) result.addr;
  UInt *u1;

  u1 = (UInt*)args[0].addr;

  res->SetDefined(u1->IsDefined());
  if ( u1->IsDefined() )
    {
      res->timeInterval.CopyFrom(u1->timeInterval);
      res->a = 0.0;
      res->b = 0.0;
      res->c = u1->constValue.GetIntval();
      res->r = false;
    }
  return 0;
}
/*
5.36.3 Specification for operator ~uint2ureal~

*/
const string TUuint2urealSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>uint -> ureal\n</text--->"
  "<text>uint2ureal( _ )</text--->"
  "<text>The operator creates a ureal value from an uint value.</text--->"
  "<text>query uint2ureal([const uint value ((\"2010-11-11\" "
  "\"2011-01-03\" TRUE FALSE) -5.4)])</text--->"
  ") )";

/*
5.36.4 Selection Function of operator ~uint2ureal~

We use Operator::SimpleSelect here.

*/

/*
5.36.5 Definition of operator ~uint2ureal~

*/
Operator temporalunituint2ureal
(
 "uint2ureal",
 TUuint2urealSpec,
 TUuint2urealValueMap,
 Operator::SimpleSelect,
 TUuint2urealTypeMap
 );

/*
5.37 Operator

----
      inside:
        OK  +      upoint x uregion --> (stream ubool)
        OK  +      upoint x    line --> (stream ubool)
        pre +      upoint x  points --> (stream ubool)
        pre +     uregion x  points --> (stream ubool)

----


*/

/*
5.37.1 Type mapping function for ~inside~

*/
ListExpr TemporalUnitInsideTypeMap( ListExpr args )
{
  ListExpr arg1, arg2;
  string argstr;

  if( nl->ListLength( args ) == 2 )
    {
      arg1 = nl->First( args );
      arg2 = nl->Second( args );

      if( nl->IsEqual( arg1, UPoint::BasicType() )
        && nl->IsEqual( arg2, URegion::BasicType()) )
        return  nl->TwoElemList(nl->SymbolAtom( Symbol::STREAM() ),
                                nl->SymbolAtom( UBool::BasicType() ));
      if( nl->IsEqual( arg1, UPoint::BasicType() )
        && nl->IsEqual( arg2, Line::BasicType()) )
        return nl->TwoElemList(nl->SymbolAtom( Symbol::STREAM() ),
                               nl->SymbolAtom( UBool::BasicType() ));
      if( nl->IsEqual( arg1, UPoint::BasicType() )
        && nl->IsEqual( arg2, Points::BasicType()) )
        return nl->TwoElemList(nl->SymbolAtom( Symbol::STREAM() ),
                               nl->SymbolAtom( UBool::BasicType() ));
      if( nl->IsEqual( arg1, URegion::BasicType() )
        && nl->IsEqual( arg2, Points::BasicType()) )
        return nl->TwoElemList(nl->SymbolAtom( Symbol::STREAM() ),
                               nl->SymbolAtom( UBool::BasicType() ));
    }

  // Error case:
  nl->WriteToString(argstr, args);
  ErrorReporter::ReportError(
    "Operator inside expects a list of length two with a certain signature. "
    "But it gets '" + argstr + "'.");
  return nl->SymbolAtom(Symbol::TYPEERROR());
}

/*
5.37.2 Value mapping for operator ~inside~

*/
struct TUInsideLocalInfo
{
  bool finished;
  int  NoOfResults;
  int  NoOfResultsDelivered;
  MBool *mbool;  // Used to store results
};
// case      upoint x uregion --> (stream ubool)
int temporalUnitInside_up_ur( Word* args, Word& result, int message,
                                    Word& local, Supplier s )
{
  // This implementation uses class-function
  //      void MRegion::Inside(MPoint& mp, MBool& res)
  TUInsideLocalInfo *sli;
  Word    a0, a1;
  UPoint  *u;
  URegion *r;
  UBool   cu;

  switch( message )
    {
    case OPEN:

#ifdef TUA_DEBUG
      cerr << "temporalUnitInside_up_ur: Received OPEN" << endl;
#endif

      sli = new TUInsideLocalInfo;
      sli->finished = true;
      sli->NoOfResults = 0;
      sli->NoOfResultsDelivered = 0;
      sli->mbool = new MBool(10);
      local.setAddr(sli);

      // initialize arguments, such that a0 always contains the upoint
      //                       and a1 the uregion
      a0 = args[0];
      a1 = args[1];
      u = (UPoint*)(a0.addr);
      r = (URegion*)(a1.addr);

      // test for definedness
      if ( !u->IsDefined() || !r->IsDefined() )
        {
#ifdef TUA_DEBUG
          cerr << "  Undef arg -> Empty Result" << endl << endl;
#endif
          // nothing to do
        }
      else
        {
          MPoint  mp_tmp(1);
          mp_tmp.Clear();         // create temporary MPoint
          mp_tmp.SetDefined( true );
          mp_tmp.Add(*u);

          MRegion mr_tmp(1);
          mr_tmp.Clear();         // create temporary MRegion
          mr_tmp.SetDefined( true );
          mr_tmp.AddURegion(*r);

          mr_tmp.Inside(mp_tmp, *(sli->mbool)); // get and save result;

          sli->NoOfResults = sli->mbool->GetNoComponents();
          sli->finished = (sli->NoOfResults <= 0);
#ifdef TUA_DEBUG
          cerr << "  " << sli->NoOfResults << " result units" << endl << endl;
#endif
        }
#ifdef TUA_DEBUG
      cerr << "temporalUnitInside_up_ur: Finished OPEN"
           << endl;
#endif
      return 0;

    case REQUEST:
#ifdef TUA_DEBUG
      cerr << "temporalUnitInside_up_ur: Received REQUEST"<< endl;
#endif
      if(local.addr == 0)
        {
#ifdef TUA_DEBUG
          cerr << "temporalUnitInside_up_ur: Finished REQUEST (1)" << endl;
#endif
          return CANCEL;
        }
      sli = (TUInsideLocalInfo*) local.addr;
      if(sli->finished)
        {
#ifdef TUA_DEBUG
          cerr << "temporalUnitInside_up_ur: Finished REQUEST (2)"<< endl;
#endif
          return CANCEL;
        }
      if(sli->NoOfResultsDelivered < sli->NoOfResults)
        {
          sli->mbool->Get(sli->NoOfResultsDelivered, cu);
          result.setAddr( cu.Clone() );
          sli->NoOfResultsDelivered++;
#ifdef TUA_DEBUG
          cerr << "temporalUnitInside_up_ur: "
               << "Finished REQUEST (YIELD)" << endl;
#endif
          return YIELD;
        }
      sli->finished = true;
#ifdef TUA_DEBUG
      cerr << "temporalUnitInside_up_ur: Finished REQUEST (3)"<< endl;
#endif
      return CANCEL;

    case CLOSE:

#ifdef TUA_DEBUG
      cerr << "temporalUnitInside_up_ur: Received CLOSE"<< endl;
#endif
      if (local.addr != 0)
        {
          sli = (TUInsideLocalInfo*) local.addr;
          delete sli->mbool;
          delete sli;
          local.setAddr(0);
        }
#ifdef TUA_DEBUG
      cerr << "temporalUnitInside_up_ur: Finished CLOSE"<< endl;
#endif
      return 0;
    } // end switch

  cerr << "temporalUnitInside_up_ur: Received UNKNOWN COMMAND"
       << endl;
  return 0;
}

// case      upoint x    line --> (stream ubool)
int temporalUnitInside_up_l( Word* args, Word& result, int message,
                                    Word& local, Supplier s )
{
  cout << "\nATTENTION: temporalUnitInside_up_l "
       << "not yet implemented!" << endl;
  assert( false );
  return 0;
}

// case      upoint x  points --> (stream ubool)
int temporalUnitInside_up_pts( Word* args, Word& result, int message,
                                    Word& local, Supplier s )
{
  cout << "\nATTENTION: temporalUnitInside_up_pts "
       << "not yet implemented!" << endl;
  assert( false );
  return 0;
}

// case      uregion x  points --> (stream ubool)
int temporalUnitInside_ur_pts( Word* args, Word& result, int message,
                                    Word& local, Supplier s )
{
  cout << "\nATTENTION: temporalUnitInside_ur_pts "
       << "not yet implemented!" << endl;
  assert( false );
  return 0;
}

/*
5.37.3 Specification for operator ~inside~

*/

const string  TemporalUnitInsideSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "("
  "<text>"
  "(upoint  uregion) -> (stream ubool)\n"
  "(upoint     line) -> (stream ubool)*\n"
  "(upoint   points) -> (stream ubool)*\n"
  "(uregion  points) -> (stream ubool)*\n"
  "(*):  Not yet implemented</text--->"
  "<text>_ inside _</text--->"
  "<text>Returns a stream of ubool indicating, whether the first "
  "object is fully included by the second one.</text--->"
  "<text>query upoint1 inside uregion1 count</text--->"
  ") )";

/*
5.37.4 Selection Function of operator ~inside~

*/
ValueMapping temporalunitinsidemap[] =
  {
    temporalUnitInside_up_ur,
    temporalUnitInside_up_l,
    temporalUnitInside_up_pts,
    temporalUnitInside_ur_pts
  };

int temporalunitInsideSelect( ListExpr args )
{
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  if( nl->IsEqual( arg1, UPoint::BasicType() )   &&
      nl->IsEqual( arg2, URegion::BasicType()) )   return 0;
  if( nl->IsEqual( arg1, UPoint::BasicType() )    &&
      nl->IsEqual( arg2, Line::BasicType() ) )    return 1;
  if( nl->IsEqual( arg1, UPoint::BasicType() )   &&
      nl->IsEqual( arg2, Points::BasicType() ) )   return 2;
  if( nl->IsEqual( arg1, URegion::BasicType() )  &&
      nl->IsEqual( arg2, Points::BasicType() ) )  return 3;

  cerr << "ERROR: Unmatched case in temporalunitInsideSelect" << endl;
  string argstr;
  nl->WriteToString(argstr, args);
  cerr << "       Argumets = '" << argstr << "'." << endl;
  return -1;
}

/*
5.37.5 Definition of operator ~inside~

*/
Operator temporalunitinside( "inside",
                             TemporalUnitInsideSpec,
                             4,
                             temporalunitinsidemap,
                             temporalunitInsideSelect,
                             TemporalUnitInsideTypeMap);

/*
5.38 Operator ~sometimes~

----

 sometimes: (       ubool) --> bool
            (stream ubool) --> bool

----

*/

/*
5.38.1 Type mapping function for ~sometimes~

*/
ListExpr TemporalUnitBoolAggrTypeMap( ListExpr args )
{
  ListExpr t;
  string argstr;

  if ( nl->ListLength( args ) == 1 )
    {
      if (nl->IsAtom(nl->First(args)))
        t = nl->First( args );
      else if (nl->ListLength(nl->First(args))==2 &&
               nl->IsEqual(nl->First(nl->First(args)), Symbol::STREAM()))
        t = nl->Second(nl->First(args));
      else
        {
          ErrorReporter::ReportError
            ("Operator sometimes/always/never expects a (stream ubool)"
             "or (ubool).");
          return nl->SymbolAtom( Symbol::TYPEERROR() );
        }

      if( nl->IsEqual( t, UBool::BasicType() ) )
        return nl->SymbolAtom( CcBool::BasicType() );
    }
  nl->WriteToString(argstr, args);
  ErrorReporter::ReportError
    ("Operator sometimes/always/never expects a list of length one, "
     "having list structure (ubool) or (stream ubool), but it gets '"
     + argstr + "'.");
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

/*
5.38.2 Value mapping for operator ~sometimes~

*/

int TemporalUnitSometimes_streamubool( Word* args, Word& result, int message,
                                       Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  CcBool *res = (CcBool*) result.addr;
  Word elem;
  UBool *U;
  bool found = false;

  qp->Open(args[0].addr);              // open input stream
  qp->Request(args[0].addr, elem);     // get first elem from stream
  while ( !found && qp->Received(args[0].addr) )
    {
      U = (UBool*) elem.addr;
      if ( U->IsDefined() && U->constValue.GetBoolval() ){
        found = true;
      } else {
        qp->Request(args[0].addr, elem);
      }
      U->DeleteIfAllowed();
    }
  qp->Close(args[0].addr); // close the stream

  // create and return the result
  res->Set( true, found );
  return 0;
}

int TemporalUnitSometimes_ubool( Word* args, Word& result, int message,
                                 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  CcBool *res = (CcBool*) result.addr;
  UBool *U = (UBool*) args[0].addr;
  res->Set( true, ( U->IsDefined() && U->constValue.GetBoolval() ) );
  return 0;
}
/*
5.38.3 Specification for operator ~sometimes~

*/

const string  TemporalUnitSometimesSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "("
  "<text>"
  "ubool -> bool\n"
  "(stream ubool) -> bool\n"
  "</text--->"
  "<text>sometimes( _ )</text--->"
  "<text>Returns 'true', iff the ubool/stream of ubool is 'true'"
  "at least once, otherwise 'false'. Never returns 'undef' itself.</text--->"
  "<text>query sometimes(ubool1)</text--->"
  ") )";

/*
5.38.4 Selection Function of operator ~sometimes~

*/

int
TemporalUnitBoolAggrSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );

  if (nl->IsAtom( arg1 ) )
    if( nl->SymbolValue( arg1 ) == UBool::BasicType() )
      return 0;
  if(   !( nl->IsAtom( arg1 ) )
      && ( nl->ListLength(arg1) == 2 )
      && ( TypeOfRelAlgSymbol(nl->First(arg1)) == stream ) )
    { if( nl->IsEqual( nl->Second(arg1), UBool::BasicType() ) )
        return 1;
    }
  cerr << "Problem in TemporalUnitBoolAggrSelect!";
  return -1; // This point should never be reached
}

ValueMapping TemporalUnitSometimesMap[] = {
  TemporalUnitSometimes_ubool,
  TemporalUnitSometimes_streamubool,
};

/*
5.38.5 Definition of operator ~sometimes~

*/
Operator temporalunitsometimes( "sometimes",
                                TemporalUnitSometimesSpec,
                                2,
                                TemporalUnitSometimesMap,
                                TemporalUnitBoolAggrSelect,
                                TemporalUnitBoolAggrTypeMap);

/*
5.38 Operator ~never~

----

     never: (       ubool) --> bool
            (stream ubool) --> bool

----

*/

/*
5.39.1 Type mapping function for ~never~

Using ~TemporalUnitBoolAggrTypeMap~

*/

/*
5.39.2 Value mapping for operator ~never~localinfo

*/
int TemporalUnitNever_streamubool( Word* args, Word& result, int message,
                                       Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  CcBool *res = (CcBool*) result.addr;
  Word elem;
  UBool *U;
  bool found = true;

  qp->Open(args[0].addr);              // open input stream
  qp->Request(args[0].addr, elem);     // get first elem from stream
  while ( found && qp->Received(args[0].addr) )
    {
      U = (UBool*) elem.addr;
      if ( U->IsDefined() && U->constValue.GetBoolval() )
        found = false;
      else
        qp->Request(args[0].addr, elem);
      U->DeleteIfAllowed();
    }
  qp->Close(args[0].addr); // close the stream

  // create and return the result
  res->Set( true, found );
  return 0;
}

int TemporalUnitNever_ubool( Word* args, Word& result, int message,
                                 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  CcBool *res = (CcBool*) result.addr;
  UBool *U = (UBool*) args[0].addr;
  res->Set( true, ( !U->IsDefined() || !U->constValue.GetBoolval() ) );
  return 0;
}
/*
5.39.3 Specification for operator ~never~

*/
const string  TemporalUnitNeverSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "("
  "<text>"
  "ubool -> bool\n"
  "(stream ubool) -> bool\n"
  "</text--->"
  "<text>never( _ )</text--->"
  "<text>Returns 'true', iff the ubool/stream does never take value 'true', "
  "otherwise 'false'. Never returns 'undef'.</text--->"
  "<text>query never(ubool1)</text--->"
  ") )";

/*
5.39.4 Selection Function of operator ~never~

Using ~TemporalUnitBoolAggrSelect~

*/
ValueMapping TemporalUnitNeverMap[] = {
  TemporalUnitNever_ubool,
  TemporalUnitNever_streamubool,
};


/*
5.39.5 Definition of operator ~never~

*/
Operator temporalunitnever( "never",
                            TemporalUnitNeverSpec,
                            2,
                            TemporalUnitNeverMap,
                            TemporalUnitBoolAggrSelect,
                            TemporalUnitBoolAggrTypeMap);

/*
5.40 Operator ~always~

----

    always: (       ubool) --> bool
            (stream ubool) --> bool

----

*/

/*
5.40.1 Type mapping function for ~always~

Using ~TemporalUnitBoolAggrTypeMap~

*/

/*
5.40.2 Value mapping for operator ~always~

*/
int TemporalUnitAlways_streamubool( Word* args, Word& result, int message,
                                       Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  CcBool *res = (CcBool*) result.addr;
  Word elem;
  UBool *U;
  bool found = true;

  qp->Open(args[0].addr);              // open input stream
  qp->Request(args[0].addr, elem);     // get first elem from stream
  while ( found && qp->Received(args[0].addr) )
    {
      U = (UBool*) elem.addr;
      if ( U->IsDefined() && !U->constValue.GetBoolval() )
        found = false;
      else
        qp->Request(args[0].addr, elem);
      U->DeleteIfAllowed();
    }
  qp->Close(args[0].addr); // close the stream

  // create and return the result
  res->Set( true, found );
  return 0;
}

int TemporalUnitAlways_ubool( Word* args, Word& result, int message,
                                 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  CcBool *res = (CcBool*) result.addr;
  UBool *U = (UBool*) args[0].addr;
  res->Set( true, ( U->IsDefined() && U->constValue.GetBoolval() ) );
  return 0;
}
/*
5.40.3 Specification for operator ~always~

*/
const string  TemporalUnitAlwaysSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "("
  "<text>"
  "ubool -> bool\n"
  "(stream ubool) -> bool"
  "</text--->"
  "<text>always( _ )</text--->"
  "<text>Returns 'false', iff the ubool/stream takes value 'false' "
  "at least once, otherwise 'true'. Never returns 'undef'.</text--->"
  "<text>query never(ubool1)</text--->"
  ") )";

/*
5.40.4 Selection Function of operator ~always~

Using ~TemporalUnitBoolAggrSelect~

*/
ValueMapping TemporalUnitAlwaysMap[] = {
  TemporalUnitAlways_ubool,
  TemporalUnitAlways_streamubool,
};

/*
5.40.5 Definition of operator ~always~

*/
Operator temporalunitalways( "always",
                             TemporalUnitAlwaysSpec,
                             2,
                             TemporalUnitAlwaysMap,
                             TemporalUnitBoolAggrSelect,
                             TemporalUnitBoolAggrTypeMap);

/*
5.41 Operator ~the\_unit~

----

   the_unit:  For T in {bool, int, string}
              point  point  instant instant bool bool --> upoint
              ipoint ipoint bool    bool              --> upoint
              real real real bool instant instant bool bool --> ureal
              iT duration bool bool --> uT
              T instant instant bool bool --> uT

----

*/

/*
5.41.1 Type mapping function for ~the\_unit~

*/
ListExpr TU_TM_TheUnit( ListExpr args )
{
  if (nl->Equal(args  , nl->SixElemList(nl->SymbolAtom(Point::BasicType()),
                                        nl->SymbolAtom(Point::BasicType()),
                                        nl->SymbolAtom(Instant::BasicType()),
                                        nl->SymbolAtom(Instant::BasicType()),
                                        nl->SymbolAtom(CcBool::BasicType()),
                                        nl->SymbolAtom(CcBool::BasicType())))){
    return nl->SymbolAtom( UPoint::BasicType() );
  }

  if (nl->Equal(args  , nl->FourElemList(nl->SymbolAtom(IPoint::BasicType()),
                                         nl->SymbolAtom(IPoint::BasicType()),
                                         nl->SymbolAtom(CcBool::BasicType()),
                                         nl->SymbolAtom(CcBool::BasicType())))){
    return nl->SymbolAtom( UPoint::BasicType() );
  }

  if(nl->Equal(args  ,nl->Cons(       nl->SymbolAtom(CcReal::BasicType()),
                      nl->Cons(       nl->SymbolAtom(CcReal::BasicType()),
                      nl->SixElemList(nl->SymbolAtom(CcReal::BasicType()),
                                      nl->SymbolAtom(CcBool::BasicType()),
                                      nl->SymbolAtom(Instant::BasicType()),
                                      nl->SymbolAtom(Instant::BasicType()),
                                      nl->SymbolAtom(CcBool::BasicType()),
                                      nl->SymbolAtom(CcBool::BasicType())))))){
    return nl->SymbolAtom( UReal::BasicType() );
  }

  if (nl->Equal(args  , nl->FiveElemList(nl->SymbolAtom(CcBool::BasicType()),
                                         nl->SymbolAtom(Instant::BasicType()),
                                         nl->SymbolAtom(Instant::BasicType()),
                                         nl->SymbolAtom(CcBool::BasicType()),
                                         nl->SymbolAtom(CcBool::BasicType())))){
    return nl->SymbolAtom( UBool::BasicType() );
  }

  if (nl->Equal(args  , nl->FourElemList(nl->SymbolAtom(IBool::BasicType()),
                                         nl->SymbolAtom(Duration::BasicType()),
                                         nl->SymbolAtom(CcBool::BasicType()),
                                         nl->SymbolAtom(CcBool::BasicType())))){
    return nl->SymbolAtom( UBool::BasicType() );
  }

  if (nl->Equal(args  , nl->FiveElemList(nl->SymbolAtom(CcInt::BasicType()),
                                         nl->SymbolAtom(Instant::BasicType()),
                                         nl->SymbolAtom(Instant::BasicType()),
                                         nl->SymbolAtom(CcBool::BasicType()),
                                         nl->SymbolAtom(CcBool::BasicType())))){
    return nl->SymbolAtom( UInt::BasicType() );
  }

  if (nl->Equal(args  , nl->FourElemList(nl->SymbolAtom(IInt::BasicType()),
                                         nl->SymbolAtom(Duration::BasicType()),
                                         nl->SymbolAtom(CcBool::BasicType()),
                                         nl->SymbolAtom(CcBool::BasicType())))){
    return nl->SymbolAtom( UInt::BasicType() );
  }

  if (nl->Equal(args  , nl->FiveElemList(nl->SymbolAtom(CcString::BasicType()),
                                         nl->SymbolAtom(Instant::BasicType()),
                                         nl->SymbolAtom(Instant::BasicType()),
                                         nl->SymbolAtom(CcBool::BasicType()),
                                         nl->SymbolAtom(CcBool::BasicType())))){
    return nl->SymbolAtom( UString::BasicType() );
  }

  if (nl->Equal(args  , nl->FourElemList(nl->SymbolAtom(IString::BasicType()),
                                         nl->SymbolAtom(Duration::BasicType()),
                                         nl->SymbolAtom(CcBool::BasicType()),
                                         nl->SymbolAtom(CcBool::BasicType())))){
    return nl->SymbolAtom( UString::BasicType() );
  }

  return listutils::typeError(
     "Operator 'the_unit' expects a list with structure\n"
     "'(point point instant instant bool bool)', or \n"
     "'(ipoint ipoint bool bool)', or \n"
     "'(real real real bool instant instant bool bool)', or\n"
     "'(T instant instant bool bool)', or \n"
     "'(iT duration bool bool)'\n for T in {bool, int, string}.");
}

/*
5.41.2 Value mapping for operator ~the\_unit~

*/
// point point instant instant bool bool --> ubool
int TU_VM_TheUnit_ppiibb(Word* args, Word& result,
                           int message, Word& local, Supplier s)
{
  result = (qp->ResultStorage( s ));
  UPoint *res   = (UPoint*)result.addr;
  Point *p1 = (Point*)args[0].addr;
  Point *p2 = (Point*)args[1].addr;
  Instant *i1 = (Instant*)args[2].addr;
  Instant *i2 = (Instant*)args[3].addr;
  CcBool *cl = (CcBool*)args[4].addr;
  CcBool *cr = (CcBool*)args[5].addr;
  bool clb, crb;

  // Test arguments for definedness
  if ( !p1->IsDefined() || !p2->IsDefined() ||
       !i1->IsDefined() || !i2->IsDefined() ||
       !cl->IsDefined() || !cr->IsDefined()    )
  {
    res->SetDefined( false );
    return 0;
  }
  clb = cl->GetBoolval();
  crb = cr->GetBoolval();

  if ( ( (*i1 == *i2) && (!clb || !crb) ) ||
       ( i1->Adjacent(i2) && !(clb || crb) ) )
    // illegal interval setting
    { res->SetDefined( false ); return 0; }
  if ( *i1 < *i2 ) // sort instants
  {
    Interval<Instant> interval( *i1, *i2, clb, crb );
    *res = UPoint( interval, *p1, *p2 );
  }
  else
  {
    Interval<Instant> interval( *i2, *i1, clb, crb );
    *res = UPoint( interval, *p2, *p1 );
  }
  return 0;
}

// ipoint ipoint bool bool --> upoint
int TU_VM_TheUnit_ipipbb(Word* args, Word& result,
                           int message, Word& local, Supplier s)
{
  result = (qp->ResultStorage( s ));
  UPoint *res   = (UPoint*)result.addr;
  Intime<Point> *ip1 = (Intime<Point> *)args[0].addr;
  Intime<Point> *ip2 = (Intime<Point> *)args[1].addr;
  CcBool *cl = (CcBool*)args[2].addr;
  CcBool *cr = (CcBool*)args[3].addr;
  bool clb, crb;

  // Test arguments for definedness
  if ( !ip1->IsDefined() || !ip2->IsDefined() ||
       !cl->IsDefined() || !cr->IsDefined()    )
  {
    res->SetDefined( false );
    return 0;
  }
  clb = cl->GetBoolval();
  crb = cr->GetBoolval();

  if ( ( (ip1->instant == ip2->instant) && (!clb || !crb) ) ||
       ( ip1->instant.Adjacent(&ip2->instant) && !(clb || crb) ) )
    // illegal interval setting
    { res->SetDefined( false ); return 0; }
  if ( ip1->instant < ip2->instant ) // sort instants
  {
    Interval<Instant> interval( ip1->instant, ip2->instant, clb, crb );
    *res = UPoint( interval, ip1->value, ip2->value );
  }
  else
  {
    Interval<Instant> interval( ip2->instant, ip1->instant, clb, crb );
    *res = UPoint( interval, ip2->value, ip1->value );
  }
  return 0;
}

// real real real bool instant instant bool bool -> ubool
int TU_VM_TheUnit_rrrbiibb(Word* args, Word& result,
                           int message, Word& local, Supplier s)
{
  result = (qp->ResultStorage( s ));
  UReal *res   = (UReal*)result.addr;
  CcReal *a = (CcReal*)args[0].addr;
  CcReal *b = (CcReal*)args[1].addr;
  CcReal *c = (CcReal*)args[2].addr;
  CcBool *r = (CcBool*)args[3].addr;
  Instant *i1 = (Instant*)args[4].addr;
  Instant *i2 = (Instant*)args[5].addr;
  CcBool *cl = (CcBool*)args[6].addr;
  CcBool *cr = (CcBool*)args[7].addr;
  bool clb, crb;

  // Test arguments for definedness
  if ( !a->IsDefined() || !b->IsDefined() ||
       !c->IsDefined() || !r->IsDefined() ||
       !i1->IsDefined() || !i2->IsDefined() ||
       !cl->IsDefined() || !cr->IsDefined()    )
  {
    res->SetDefined( false );
    return 0;
  }
  clb = cl->GetBoolval();
  crb = cr->GetBoolval();

  if ( ( (*i1 == *i2) && (!clb || !crb) )    ||
       (  i1->Adjacent(i2) && !(clb || crb) )  )// illegal interval setting
    { res->SetDefined( false ); return 0; }
  if ( *i1 < *i2 ) // sort instants
  {
    Interval<Instant> interval( *i1, *i2, clb, crb );
    *res = UReal( interval, a->GetRealval(), b->GetRealval(),
                            c->GetRealval(), r->GetBoolval() );
  }
  else
  {
    Interval<Instant> interval( *i2, *i1, clb, crb );
    *res = UReal( interval, a->GetRealval(), b->GetRealval(),
                            c->GetRealval(), r->GetBoolval() );
  };
  return 0;
}

// template for constant unit types:
// iT duration bool bool -> uT
template<class T>
int TU_VM_TheUnit_iTdbb(Word* args, Word& result,
                        int message, Word& local, Supplier s)
{
  result = (qp->ResultStorage( s ));
  ConstTemporalUnit<T> *res   = (ConstTemporalUnit<T> *)result.addr;
  Intime<T> *ip = (Intime<T> *)args[0].addr;
  DateTime *dur = (DateTime*) args[1].addr;
  CcBool *cl = (CcBool*)args[2].addr;
  CcBool *cr = (CcBool*)args[3].addr;
  bool clb, crb;

  // Test arguments for definedness
  if ( !ip->IsDefined() || !dur->IsDefined() ||
       !cl->IsDefined() || !cr->IsDefined()    )
  {
    res->SetDefined( false );
    return 0;
  }
  clb = cl->GetBoolval();
  crb = cr->GetBoolval();

  assert(dur->GetType() == durationtype);
  if ( ( (*dur == DateTime(0,0,durationtype)) &&  (!clb || !crb) ) ||
       ( (*dur == DateTime(0,1,durationtype)) && !( clb ||  crb) )    )
    // illegal interval setting
    { res->SetDefined( false ); return 0; }
  Interval<Instant> interval( ip->instant, ip->instant+(*dur), clb, crb );
  *res = ConstTemporalUnit<T>( interval, ip->value );
  return 0;
}


// template for constant unit types:
// T instant instant bool bool -> uT
template<class T>
int TU_VM_TheUnit_Tiibb(Word* args, Word& result,
                        int message, Word& local, Supplier s)
{
  result = (qp->ResultStorage( s ));
  ConstTemporalUnit<T> *res = static_cast<ConstTemporalUnit<T> *>(result.addr);
  T       *value = static_cast<T*>(args[0].addr);
  Instant *i1    = static_cast<DateTime*>(args[1].addr);
  Instant *i2    = static_cast<DateTime*>(args[2].addr);
  CcBool  *cl    = static_cast<CcBool*>(args[3].addr);
  CcBool  *cr    = static_cast<CcBool*>(args[4].addr);
  bool clb, crb;

  // Test arguments for definedness
  if ( !value->IsDefined() ||
       !i1->IsDefined() || !i2->IsDefined() ||
       !cl->IsDefined() || !cr->IsDefined()    ) {
    res->SetDefined( false );
    return 0;
  }

  clb = cl->GetBoolval();
  crb = cr->GetBoolval();

  if ( ( (*i1 == *i2) && (!clb || !crb) )   ||
       ( i1->Adjacent(i2) && !(clb || crb) )  ) { // illegal interval setting
    res->SetDefined( false );
    return 0;
  }
  if ( *i1 < *i2 ) {// sort instants
    Interval<Instant> interval( *i1, *i2, clb, crb );
    *res = ConstTemporalUnit<T>( interval, *value );
  } else {
    Interval<Instant> interval( *i2, *i1, clb, crb );
    *res = ConstTemporalUnit<T>( interval, *value );
  }
  return 0;
}


/*
5.41.3 Specification for operator ~the\_unit~

*/
const string  TU_Spec_TheUnit =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "("
  "<text>For T in {bool, int, string}:\n"
  "point x point x instant x instant x bool x bool --> upoint\n"
  "ipoint x ipoint x bool x bool --> upoint\n"
  "real x real x real x bool x instant x instant x bool x bool --> ureal\n"
  "iT x duration x bool x bool --> uT\n"
  "T x instant x instant x bool x bool --> uT"
  "</text--->"
  "<text>the_unit( pstart, pend, tstart, tend, cl, cr )\n"
  "the_unit( ip1, ip2, cl, cr )\n"
  "the_unit( a, b, c, r, tstart, tend, cl, cr )</text--->"
  "<text>Creates a unit value from the argument list. The instants/ipoints"
  "/(point/instant)-pairs will be sorted automatically.</text--->"
  "<text>query the_unit(point1, point2, instant1, instant2, bool1, bool2)"
  "</text--->"
  ") )";

/*
5.41.4 Selection Function of operator ~the\_unit~

*/
int TU_Select_TheUnit( ListExpr args )
{
  string argstr;
  nl->WriteToString(argstr, args);

  if (argstr == "(point point instant instant bool bool)")
    return 0;
  if (argstr == "(ipoint ipoint bool bool)")
    return 1;

  if (argstr == "(real real real bool instant instant bool bool)")
    return 2;

  if (argstr == "(bool instant instant bool bool)")
    return 3;
  if (argstr == "(ibool duration bool bool)")
    return 4;

  if (argstr == "(int instant instant bool bool)")
    return 5;
  if (argstr == "(iint duration bool bool)")
    return 6;

  if (argstr == "(string instant instant bool bool)")
    return 7;
  if (argstr == "(istring duration bool bool)")
    return 8;

  return -1; // should not be reached!
}

ValueMapping TU_VMMap_TheUnit[] =
  {
    TU_VM_TheUnit_ppiibb,           //0
    TU_VM_TheUnit_ipipbb,
    TU_VM_TheUnit_rrrbiibb,         //2
    TU_VM_TheUnit_Tiibb<CcBool>,
    TU_VM_TheUnit_iTdbb<CcBool>,    //4
    TU_VM_TheUnit_Tiibb<CcInt>,
    TU_VM_TheUnit_iTdbb<CcInt>,     //6
    TU_VM_TheUnit_Tiibb<CcString>,
    TU_VM_TheUnit_iTdbb<CcString>}; //8
/*
5.41.5 Definition of operator ~the\_unit~

*/
Operator temporalunittheupoint( "the_unit",
                            TU_Spec_TheUnit,
                            9,
                            TU_VMMap_TheUnit,
                            TU_Select_TheUnit,
                            TU_TM_TheUnit);
/*
5.42 Operator ~the\_ivalue~

This operator creates an intime value from an instant and a value.

----
      the_ivalue:  For T in {bool, int, string, real, point, region}
OK                               (instant T) --> iT

----

*/

/*
5.42.1 Type mapping function for ~the\_ivalue~

*/
ListExpr TU_TM_TheIvalue( ListExpr args )
{
  if( (nl->ListLength(args)==2) &&
    (listutils::isSymbol(nl->First(args),Instant::BasicType())) ){

    ListExpr second = nl->Second(args);
    if (listutils::isSymbol(second, CcBool::BasicType()) ){
      return nl->SymbolAtom( IBool::BasicType() );
    }
    if (listutils::isSymbol(second, CcInt::BasicType()) ){
      return nl->SymbolAtom( IInt::BasicType() );
    }
    if (listutils::isSymbol(second, CcString::BasicType()) ){
      return nl->SymbolAtom( IString::BasicType() );
    }
    if (listutils::isSymbol(second, CcReal::BasicType()) ){
      return nl->SymbolAtom( IReal::BasicType() );
    }
    if (listutils::isSymbol(second, Point::BasicType()) ){
      return nl->SymbolAtom( IPoint::BasicType() );
    }
    if (listutils::isSymbol(second, Region::BasicType()) ){
      return nl->SymbolAtom( IRegion::BasicType() );
    }
  }
  return listutils::typeError(
     "Operator 'the_ivalue' expects a list with structure "
     "'(instant T)', "
     "for T in {bool, int, string, real, point, region}.");
}

/*
5.42.2 Value mapping for operator ~the\_ivalue~

*/

template <class T>
int TU_VM_TheIvalue(Word* args, Word& result,
                        int message, Word& local, Supplier s)
{
  result = (qp->ResultStorage( s ));
  Intime<T> *res   = (Intime<T> *) result.addr;
  Instant   *inst  = (DateTime*)   args[0].addr;
  T         *value = (T*)          args[1].addr;
  *res = Intime<T>(*inst, *value);
  return 0;

}

/*
5.42.3 Specification for operator ~the\_ivalue~

*/
const string  TU_Spec_TheIvalue =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "("
  "<text>For T in {bool, int, string, real, point, region}:\n"
  "instant T --> iT"
  "</text--->"
  "<text>the_ivalue( instant, value )</text--->"
  "<text>Creates an intime value from the argument list. "
  "An undef 'instant' argument results in an undef result.</text--->"
  "<text>query the_ivalue(instant1, point1)"
  "</text--->"
  ") )";

/*
5.42.4 Selection Function of operator ~the\_ivalue~

*/
int TU_Select_TheIvalue( ListExpr args )
{
  string argstr;
  nl->WriteToString(argstr, args);

  if (argstr == "(instant bool)")
    return 0;
  if (argstr == "(instant int)")
    return 1;
  if (argstr == "(instant string)")
    return 2;
  if (argstr == "(instant real)")
    return 3;
  if (argstr == "(instant point)")
    return 4;
  if (argstr == "(instant region)")
    return 5;

  return -1; // should not be reached!
}

ValueMapping TU_VMMap_TheIvalue[] =
  {
    TU_VM_TheIvalue<CcBool>,
    TU_VM_TheIvalue<CcInt>,
    TU_VM_TheIvalue<CcString>,
    TU_VM_TheIvalue<CcReal>,
    TU_VM_TheIvalue<Point>,
    TU_VM_TheIvalue<Region>
  };
/*
5.42.5 Definition of operator ~the\_ivalue~

*/
Operator temporalunittheivalue( "the_ivalue",
                            TU_Spec_TheIvalue,
                            6,
                            TU_VMMap_TheIvalue,
                            TU_Select_TheIvalue,
                            TU_TM_TheIvalue);

/*
5.41 Operator ~ComparePredicateValues~

Here, we implement the binary comparison operators/predicates for (uT uT).
The predicates are

----

   = (equality),
   # (unequality),
   < (smaller than),
   > (bigger than),
  <= (smaller than or equal to),
  >= (bigger than or equal to).

----

The operators compare the values for each instant of time, so they will return
a (stream ubool) for the intersection of the arguments' deftimes.

WARNING: Do not confuse this operators with the ~ComparePredicates~, which
         compare the units as such, but not the temporal functions.

----
      =, #, <, >, <=, >=: For T in {int, bool, real, string, point*, region*}
                             (*): {#, =} only
Test +        uT x uT --> (stream ubool)
Test +        uT x  T --> (stream ubool) (**)
Test +         T x uT --> (stream ubool) +(**)
                             (**): Not for T = region

----

*/

/*
5.43.1 Type mapping function for ~ComparePredicateValues~

*/
ListExpr TUCompareValuePredicatesTypeMap( ListExpr args )
{

  if( nl->ListLength( args ) == 2 )
    {
      ListExpr arg1, arg2;
      arg1 = nl->First( args );
      arg2 = nl->Second( args );
      if (nl->Equal( arg1, arg2 ))
        {
          if( (nl->IsEqual( arg1, UBool::BasicType() ) )   ||
              (nl->IsEqual( arg1, UInt::BasicType() ) )    ||
              (nl->IsEqual( arg1, UReal::BasicType() ) )   ||
              (nl->IsEqual( arg1, UString::BasicType() ) )
            )
            return nl->TwoElemList(nl->SymbolAtom( Symbol::STREAM() ),
                                nl->SymbolAtom( UBool::BasicType() ));
        }
      else if
        (
         ((nl->IsEqual( arg1, UBool::BasicType() ))
         && (nl->IsEqual( arg2, CcBool::BasicType() )))   ||
         ((nl->IsEqual( arg1, UInt::BasicType() ))
         && (nl->IsEqual( arg2, CcInt::BasicType() )))    ||
         ((nl->IsEqual( arg1, UReal::BasicType() ))
         && (nl->IsEqual( arg2, CcReal::BasicType() )))   ||
         ((nl->IsEqual( arg1, UString::BasicType() ))
         && (nl->IsEqual( arg2, CcString::BasicType() ))) ||
         ((nl->IsEqual( arg1, CcBool::BasicType() ))
         && (nl->IsEqual( arg2, UBool::BasicType() )))  ||
         ((nl->IsEqual( arg1, CcInt::BasicType() ))
         && (nl->IsEqual( arg2, UInt::BasicType() )))   ||
         ((nl->IsEqual( arg1, CcReal::BasicType() ))
         && (nl->IsEqual( arg2, UReal::BasicType() )))  ||
         ((nl->IsEqual( arg1, CcString::BasicType() ))
         && (nl->IsEqual( arg2, UString::BasicType() )))
        )
        return nl->TwoElemList(nl->SymbolAtom( Symbol::STREAM() ),
                               nl->SymbolAtom( UBool::BasicType() ));
    }
  // Error case:
  ErrorReporter::ReportError(
    "CompareTemporalValueOperator (one of <, >, <=, >=) "
    "expects two arguments of types '(uT x uT)', '(T x uT)', or '(uT x T)', "
    "where T in {bool, int, real, string}.");
  return nl->SymbolAtom(Symbol::TYPEERROR());
}

ListExpr TUCompareValueEqPredicatesTypeMap( ListExpr args )
{
  if( nl->ListLength( args ) == 2 )
    {
      ListExpr arg1, arg2;
      arg1 = nl->First( args );
      arg2 = nl->Second( args );
      if (nl->Equal( arg1, arg2 ))
        {
          if( (nl->IsEqual( arg1, UBool::BasicType() ) )   ||
              (nl->IsEqual( arg1, UInt::BasicType() ) )    ||
              (nl->IsEqual( arg1, UReal::BasicType() ) )   ||
              (nl->IsEqual( arg1, UString::BasicType() )   ||
              (nl->IsEqual( arg1, UPoint::BasicType() ) )   ||
              (nl->IsEqual( arg1, URegion::BasicType() ) ) ) )
            return nl->TwoElemList(nl->SymbolAtom( Symbol::STREAM() ),
                                nl->SymbolAtom( UBool::BasicType() ));
        }
      string argstr;
      nl->WriteToString(argstr, args);
      if (argstr == "(bool ubool)"     || argstr == "(ubool bool)"     ||
          argstr == "(int uint)"       || argstr == "(unit int)"       ||
          argstr == "(string ustring)" || argstr == "(ustring string)" ||
          argstr == "(real ureal)"     || argstr == "(ureal real)"     ||
          argstr == "(point upoint)"   || argstr == "(upoint point)")
        return nl->TwoElemList(nl->SymbolAtom( Symbol::STREAM() ),
                               nl->SymbolAtom( UBool::BasicType() ));
    }
// Error case:
  ErrorReporter::ReportError(
    "CompareTemporalValueOperator (one of =, #) "
    "expects two arguments of "
    "type 'uT x uT', 'T x uT' or 'uT x T', where T in "
    "{bool, int, real, string, point}, or (uregion x uregion). ");
  return nl->SymbolAtom(Symbol::TYPEERROR());
}

/*
5.43.2 Value mapping for operator ~ComparePredicateValues~

*/
template <class T, int opcode>
int TU_VM_ComparePredicateValue_Const(Word* args, Word& result,
                                      int message, Word& local, Supplier s)
{
  assert( (opcode >= 0) && (opcode <= 5));

  ConstTemporalUnit<T> *u1  = (ConstTemporalUnit<T> *) args[0].addr;
  ConstTemporalUnit<T> *u2  = (ConstTemporalUnit<T> *) args[1].addr;
  bool *finished;
  Interval<Instant> iv;
  int p1;
  bool compresult = false;

  switch( message )
  {
    case OPEN:
      finished = new bool(false); //
      local.setAddr(finished);
      return 0;

    case REQUEST:
      if( local.addr == 0 )
        return CANCEL;
      finished = (bool*) local.addr;
      if ( *finished )
        return CANCEL;
      if ( !u1->IsDefined() || !u2->IsDefined() )
        { *finished = true; return CANCEL; }
      if (!u1->timeInterval.Intersects(u2->timeInterval) )
      { *finished = true; return CANCEL; }

      u1->timeInterval.Intersection(u2->timeInterval, iv);
      p1 = u1->constValue.Compare( &u2->constValue );

      switch (opcode)
      {
        case 0: // is_equal
          compresult = (p1 == 0); break;
        case 1: // is_not_equal
          compresult =  (p1 != 0); break;
        case 2: // less_than
          compresult = (p1 == -1); break;
        case 3: // bigger_than
          compresult = (p1 == 1); break;
        case 4: // less_or_equal
          compresult = (p1 < 1); break;
        case 5: // bigger_or_equal
          compresult = (p1 > -1); break;
      } // end switch (opcode)
      result.setAddr(
          new ConstTemporalUnit<CcBool>(iv, CcBool(true, compresult) ) );
      *finished = true; // only one result!
      return YIELD;

    case CLOSE:
      if( local.addr != 0 )
      {
        finished = (bool*) local.addr;
        delete finished;
        local.setAddr(0);
      }
      return 0;
  } // end switch (message)
  return -1; // should not be reached
}

template <class T, int opcode, int unit_arg>
  int TU_VM_ComparePredicateValue_Const_T(Word* args, Word& result,
                                          int message, Word& local, Supplier s)
{
  assert( (opcode >= 0) && (opcode <= 5));
  assert( (unit_arg >=0) && (unit_arg <=1));

  ConstTemporalUnit<T> *u1;
  T *u2;

  if (unit_arg == 0)
  {
    u1  = (ConstTemporalUnit<T> *) args[0].addr;
    u2  = (T*) args[1].addr;
  }
  else // (unit_arg == 1)
  {
    u1  = (ConstTemporalUnit<T> *) args[1].addr;
    u2  = (T*) args[0].addr;
  }
  bool *finished;
  Interval<Instant> iv;
  int p1;
  bool compresult = false;

  switch( message )
  {
    case OPEN:
      finished = new bool(false); //
      local.setAddr(finished);
      return 0;

    case REQUEST:
      if( local.addr == 0 )
        return CANCEL;
      finished = (bool*) local.addr;
      if ( *finished )
        return CANCEL;
      if ( !u1->IsDefined() || !u2->IsDefined() )
      { *finished = true; return CANCEL; }

      if(unit_arg == 0)
        p1 = u1->constValue.Compare( u2 );
      else
        p1 = u2->Compare( &(u1->constValue) );

      switch (opcode)
      {
        case 0: // is_equal
          compresult = (p1 == 0);  break;
        case 1: // is_not_equal
          compresult =  (p1 != 0); break;
        case 2: // less_than
          compresult = (p1 == -1); break;
        case 3: // bigger_than
          compresult = (p1 == 1);  break;
        case 4: // less_or_equal
          compresult = (p1 < 1);   break;
        case 5: // bigger_or_equal
          compresult = (p1 > -1);  break;
      } // end switch (opcode)
      result.setAddr(
          new ConstTemporalUnit<CcBool>(u1->timeInterval,
                                        CcBool(true, compresult) ) );
      *finished = true; // only one result!
      return YIELD;

    case CLOSE:
      if( local.addr != 0 )
      {
        finished = (bool*) local.addr;
        delete finished;
        local.setAddr(0);
      }
      return 0;
  } // end switch (message)
  return -1; // should not be reached
}

struct TUCompareValueLocalInfo
{
  bool finished;
  int  NoOfResults;
  int  NoOfResultsDelivered;
  MBool *intersectionBool;
};

enum deltaInterpretation  {VALUE, ALWAYS, NEVER};

struct deltaV{
  deltaV(deltaInterpretation i, double v): value(v), delta(i){}
  double value;
  deltaInterpretation delta;
};

deltaV computeDelta(const double a1, const double a2, 
                    const double b1, const double b2){

  // computes delta for
  // a1 + delta*(a2-a1) = b1 + delta*(b2-b1)
  // this holds ever, never or for a certain value
  double da = a2 - a1;
  double db = b2 - b1;
  double fdelta = da-db;
  double r = b1 - a1;
  if(AlmostEqual(fdelta,0.0)){
    if(AlmostEqual(r,0.0)){
      return deltaV(ALWAYS,0.0);
    } else {
      return deltaV(NEVER,0.0);
    }
  } else {
    return deltaV(VALUE, r/fdelta);
  }
}

deltaV merge(const deltaV& d1,
                          const deltaV& d2){
   switch(d1.delta){
     case ALWAYS : return d2;
     case NEVER  : return d1;
     case VALUE  :
            switch(d2.delta){
              case ALWAYS : return d1;
              case NEVER  : return d2;
              case VALUE  : if(AlmostEqual(d1.value,d2.value)){
                              return d1;
                            } else {
                              return deltaV(NEVER,0.0); 
                            }
              default : assert(false);
            } 
     default : assert(false);
   }
   assert(false);
   return d1;
}


ostream& operator<<(ostream& o, const deltaInterpretation& i){
   switch(i){
      case ALWAYS : o << "always" ; break;
      case NEVER : o << "never" ; break;
      case VALUE : o << "value" ; break;
   }
   return o;
}
ostream& operator<<(ostream& o, const deltaV& d){
   o << d.delta;
   if(d.delta==VALUE){
     o << ":" << d.value;
   }
   return o;
}





template <int opcode>
int TU_VM_ComparePredicateValue_UPoint(Word* args, Word& result,
                                   int message, Word& local, Supplier s)
{
  assert( (opcode >= 0) && (opcode <= 1));

  UPoint *u1  = (UPoint*) args[0].addr;
  UPoint *u2  = (UPoint*) args[1].addr;
  UPoint uinters(true);
  UBool cu;
  TUCompareValueLocalInfo *localinfo;
  Interval<Instant> iv, ivBefore, ivInters, ivAfter;

  switch( message )
    {
    case OPEN: {
      localinfo = new TUCompareValueLocalInfo();
      localinfo->finished = true;
      localinfo->NoOfResults = 0;
      localinfo->NoOfResultsDelivered = 0;
      localinfo->intersectionBool = new MBool(5);
      local.setAddr(localinfo);

      if ( !u1->IsDefined() || !u2->IsDefined() ) {
          return 0;
      }
      // fill up the result
      RefinementStream<MPoint,MPoint,UPoint,UPoint> rs(u1,u2);
      int pos1; 
      int pos2;
      Interval<Instant> iv;
      bool compValue = opcode==0?true:false;

      while(rs.hasNext()){
          rs.getNext(iv,pos1,pos2);
          if(pos1==0 && pos2==0){ // the common time interval if exists
             Point p1_start(true);
             Point p1_end(true);
             Point p2_start(true);
             Point p2_end(true);
             u1->TemporalFunction(iv.start,p1_start,true);
             u1->TemporalFunction(iv.end,p1_end,true);
             u2->TemporalFunction(iv.start,p2_start,true);
             u2->TemporalFunction(iv.end,p2_end,true);
             deltaV xd = computeDelta(p1_start.GetX(), p1_end.GetX(),
                                       p2_start.GetX(), p2_end.GetX());
             deltaV yd = computeDelta(p1_start.GetY(), p1_end.GetY(),
                                      p2_start.GetY(), p2_end.GetY());
             deltaV delta = merge(xd,yd);
             switch(delta.delta){
                case ALWAYS :  localinfo->intersectionBool->
                                       Add(UBool(iv, CcBool(true,compValue)));
                               break;
                case NEVER  :   localinfo->intersectionBool->
                                       Add(UBool(iv,CcBool(true,!compValue)));
                               break;

                case VALUE  :  {
                               DateTime dur = iv.end - iv.start;
                               dur.Mul(delta.value);
                               DateTime ip(datetime::instanttype);
                               ip = iv.start + dur;
                               if( (ip < iv.start) || (ip > iv.end) ||
                                   (ip==iv.start && !iv.lc) ||
                                   (ip==iv.end && !iv.rc)){
                                  // intersection outside time interval
                                  localinfo->intersectionBool->
                                       Add(UBool(iv,CcBool(true,!compValue)));
                               } else if(ip==iv.start && ip==iv.end){
                                   // intersection covers the whole interval
                                   localinfo->intersectionBool->
                                       Add(UBool(iv,CcBool(true,compValue)));
                               } else if(ip==iv.start){
                                   // common start
                                   Interval<Instant> iv1(ip,ip,true,true);
                                   Interval<Instant> iv2(ip,iv.end,false,iv.rc);
                                   localinfo->intersectionBool->
                                       Add(UBool(iv1,CcBool(true,compValue)));
                                   localinfo->intersectionBool->
                                       Add(UBool(iv2,CcBool(true,!compValue)));
                               } else if(ip==iv.end){
                                   Interval<Instant> iv1(iv.start,ip,
                                                         iv.lc,false);
                                   Interval<Instant> iv2(ip,ip, true,true);
                                   localinfo->intersectionBool->
                                       Add(UBool(iv1,CcBool(true,!compValue)));
                                   localinfo->intersectionBool->
                                        Add(UBool(iv2,CcBool(true,compValue)));
                               } else { // ip inside iv
                                   Interval<Instant> iv1(iv.start,ip,
                                                         iv.lc,false);
                                   Interval<Instant> iv2(ip,ip,true,true);
                                   Interval<Instant> iv3(ip,iv.end,
                                                         false,iv.rc);   
                                   localinfo->intersectionBool->
                                       Add(UBool(iv1,CcBool(true,!compValue)));
                                   localinfo->intersectionBool->
                                       Add(UBool(iv2,CcBool(true,compValue)));
                                   localinfo->intersectionBool->
                                      Add(UBool(iv3,CcBool(true,!compValue)));
                               }
                               }                              
                               break;
                default     : assert(false);
             }
          }
      }
      localinfo->NoOfResults = localinfo->intersectionBool->GetNoComponents();
      localinfo->finished = localinfo->NoOfResults == 0;
      }
      return 0;
    case REQUEST:
      if( local.addr == 0 ) {
        return CANCEL;
      }
      localinfo = (TUCompareValueLocalInfo*) local.addr;
      if ( localinfo->finished ){
        return CANCEL;
      }
      if ( localinfo->NoOfResultsDelivered >= localinfo->NoOfResults) { 
         localinfo->finished = true; 
         return CANCEL;
       }
      localinfo->intersectionBool->Get(localinfo->NoOfResultsDelivered, cu);
      result.setAddr( cu.Clone() );
      localinfo->NoOfResultsDelivered++;
      return YIELD;

    case CLOSE:
      if( local.addr != 0 )
      {
        localinfo = (TUCompareValueLocalInfo*) local.addr;
        delete localinfo->intersectionBool;
        delete localinfo;
        local.setAddr(0);
      }
      return 0;
  } // end switch (message)
  return -1; // should not be reached
}

template <int opcode, int unit_arg>
    int TU_VM_ComparePredicateValue_UPoint_Point(Word* args, Word& result,
                          int message, Word& local, Supplier s)
{
  assert( (opcode >= 0) && (opcode <= 1));
  assert( (unit_arg >= 0) && (unit_arg <= 1));

  UPoint *u1;
  Point *p;

  if(unit_arg == 0)
  {
  u1  = (UPoint*) args[0].addr;
  p   = (Point*)  args[1].addr;
  }
  else{
    u1  = (UPoint*) args[1].addr;
    p   = (Point*)  args[0].addr;
  }
  UPoint *u2, uinters(true);
  UBool cu;
  TUCompareValueLocalInfo *localinfo;
  Interval<Instant> iv, ivBefore, ivInters, ivAfter;
  bool compresult = false;

  switch( message )
  {
    case OPEN:

      localinfo = new TUCompareValueLocalInfo;
      localinfo->finished = true;
      localinfo->NoOfResults = 0;
      localinfo->NoOfResultsDelivered = 0;
      localinfo->intersectionBool = new MBool(5);
      local.setAddr(localinfo);

      if ( !u1->IsDefined()) // || !u2->IsDefined() )
      {
//           cerr << "Undef input" << endl;
        return 0;
      }
      iv = u1->timeInterval;
      u2 = new UPoint(iv, *p, *p);
      u1->Intersection(*u2, uinters);
      delete u2;
      ivInters = uinters.timeInterval;
      compresult = (opcode == 0) ? uinters.IsDefined() : !uinters.IsDefined();

      if ( !uinters.IsDefined() ||
            ( uinters.IsDefined() && !uinters.timeInterval.Inside(ivInters))
         )
      {// no intersection or intersection outside common interval:
       // result unit spans common interval totally
//      cout << "No intersection in: "; iv.Print(cout); cout << endl;
        localinfo->intersectionBool->Add(UBool(iv,CcBool(true,compresult)));
        localinfo->NoOfResults++;
        localinfo->finished = false;
        return 0;
      }

      if( uinters.IsDefined() &&
          iv.start == ivInters.start &&
          iv.end == ivInters.end
        )
      {//  only one resultunit
//      cout << "Complete intersection: "; ivInters.Print(cout); cout << endl;
        localinfo->intersectionBool->Add(
            ConstTemporalUnit<CcBool>(ivInters, CcBool(true, compresult)) );
        localinfo->NoOfResults++;
        localinfo->finished = false;
        return 0;
      }

      if ( uinters.IsDefined() )
      {// possibly more than 1 resultunit
        if ( (iv.start < ivInters.start) ||
              ( (iv.start == ivInters.start) && iv.lc && !ivInters.lc &&
              ivInters.Inside(iv) )
           )
        {//  result before intersection interval
          ivBefore=Interval<Instant>(iv.start,ivInters.start,
                                     iv.lc,!ivInters.lc);
//        cout << "Before intersection: "; ivBefore.Print(cout); cout << endl;
          localinfo->intersectionBool->Add(
              ConstTemporalUnit<CcBool>(ivBefore, CcBool(true, !compresult)) );
          localinfo->NoOfResults++;
          localinfo->finished = false;
        }
        if ( ivInters.Inside(iv) )
        { // result at intersection interval
          // UPoint::Intersection(...) will also return a result being on the
          // limit of an open interval. Therefore, we need the second condition!
//        cout << "At intersection: "; ivInters.Print(cout); cout << endl;
          localinfo->intersectionBool->Add(
              ConstTemporalUnit<CcBool>(ivInters, CcBool(true, compresult)) );
          localinfo->NoOfResults++;
          localinfo->finished = false;
        }
        if ( (iv.end > ivInters.end) ||
              ( (iv.end == ivInters.end) && iv.rc && !ivInters.rc &&
              ivInters.Inside(iv) )
           )
        {//  result after intersection interval
          ivAfter = Interval<Instant>(ivInters.end,iv.end,!ivInters.rc,iv.rc);
//        cout << "After intersection: "; ivAfter.Print(cout); cout << endl;
          localinfo->intersectionBool->Add(
              ConstTemporalUnit<CcBool>(ivAfter, CcBool(true, !compresult)) );
          localinfo->NoOfResults++;
          localinfo->finished = false;
        }
      }
      return 0;

    case REQUEST:

      if( local.addr == 0 )
        return CANCEL;
      localinfo = (TUCompareValueLocalInfo*) local.addr;
      if ( localinfo->finished )
        return CANCEL;
      if ( localinfo->NoOfResultsDelivered >= localinfo->NoOfResults)
      { localinfo->finished = true; return CANCEL; }
      localinfo->intersectionBool->Get(localinfo->NoOfResultsDelivered, cu);
      result.setAddr( cu.Clone() );
      localinfo->NoOfResultsDelivered++;
      return YIELD;

    case CLOSE:
      if( local.addr != 0 )
      {
        localinfo = (TUCompareValueLocalInfo*) local.addr;
        delete localinfo->intersectionBool;
        delete localinfo;
        local.setAddr(0);
      }
      return 0;
  } // end switch (message)
  return -1; // should not be reached
}

/*
Implementation changed after ~CompUReal~ became memberfunction of UReal.
Simone

*/
template<int opcode>
    int TU_VM_ComparePredicateValue_UReal(Word* args, Word& result,
                                          int message, Word& local, Supplier s)
{
  UReal *u1  = (UReal*) args[0].addr;
  UReal *u2  = (UReal*) args[1].addr;
  TUCompareValueLocalInfo *localinfo;
  UBool cu;
  vector<UBool> res;
  switch (message)
  {
    case OPEN:
      localinfo = new TUCompareValueLocalInfo;
      local.setAddr(localinfo);
      localinfo->finished = true;
      localinfo->NoOfResults = 0;
      localinfo->NoOfResultsDelivered = 0;
      localinfo->intersectionBool = new MBool(5);
      localinfo->intersectionBool->Clear();
      if ( !u1->IsDefined() ||
            !u2->IsDefined() ||
            !u1->timeInterval.Intersects(u2->timeInterval) )
      { // no result
//      cout << "TU_VM_ComparePredicateValue_UReal: No Result." << endl;
        return 0;
      }
      // common deftime --> some result exists

      u1->CompUReal(*u2, opcode, res);
      localinfo->intersectionBool->StartBulkLoad();
      for (size_t i = 0;i < res.size();i++)
      {
        localinfo->intersectionBool->MergeAdd(res[i]);
      }
      res.clear();
      localinfo->intersectionBool->EndBulkLoad(true);
      localinfo->NoOfResults = localinfo->intersectionBool->GetNoComponents();
      localinfo->finished = ( localinfo->NoOfResults <= 0 );
      return 0;

  case REQUEST:
    if (local.addr == 0)
      return CANCEL;
    localinfo = (TUCompareValueLocalInfo*) local.addr;
    if (localinfo->finished)
      return CANCEL;
    if (localinfo->NoOfResultsDelivered >= localinfo->NoOfResults)
    {
      localinfo->finished = true;
      return CANCEL;
    }
    localinfo->intersectionBool->Get(localinfo->NoOfResultsDelivered, cu);
    result.setAddr( cu.Clone() );
    localinfo->NoOfResultsDelivered++;
    return YIELD;

  case CLOSE:
    if( local.addr != 0 )
    {
      localinfo = (TUCompareValueLocalInfo*) local.addr;
      delete localinfo->intersectionBool;
      delete localinfo;
      local.setAddr(0);
    }
  }
  return -1;
}

template<int opcode, int unit_arg>
    int TU_VM_ComparePredicateValue_UReal_CcReal(Word* args, Word& result,
                            int message, Word& local, Supplier s)
{
  assert(opcode>=0 && opcode<=5);
  assert(unit_arg>=0 && unit_arg <=1);

  UReal *u1;
  CcReal *r;

  if(unit_arg == 0)
  {
    u1  = (UReal*)  args[0].addr;
    r   = (CcReal*) args[1].addr;
  }
  else
  {
    u1 = (UReal*)  args[1].addr;
    r  = (CcReal*) args[0].addr;
  }

  UBool cu;
  UBool newunit(true);
  TUCompareValueLocalInfo *localinfo;
  Interval<Instant>
      iv(DateTime(0,0,instanttype), DateTime(0,0,instanttype), false, false),
  ivnew(DateTime(0,0,instanttype), DateTime(0,0,instanttype), false, false);
  Interval<Instant> actIntv;
  Instant
      start(instanttype),
  end(instanttype),
  testInst(instanttype);
  Periods *eqPeriods;
  int i, numEq, cmpres;
  bool compresult, lc;
  CcReal fccr1(true, 0.0), fccr2(true,0.0);

  switch (message)
  {
    case OPEN:
      localinfo = new TUCompareValueLocalInfo;
      local.setAddr(localinfo);
      localinfo->finished = true;
      localinfo->NoOfResults = 0;
      localinfo->NoOfResultsDelivered = 0;
      localinfo->intersectionBool = new MBool(5);
      localinfo->intersectionBool->Clear();

      if ( !u1->IsDefined() || !r->IsDefined() )
      { // no result
//      cout << "TU_VM_ComparePredicateValue_UReal: No Result." << endl;
        return 0;
      }
      iv = u1->timeInterval;
      eqPeriods = new Periods(4);
      u1->PeriodsAtVal(r->GetRealval(), *eqPeriods);// only intervals of length
      numEq = eqPeriods->GetNoComponents();// 1 instant herein (start==end)
//    cout << "  numEq=" << numEq << endl;
      if ( numEq == 0 )
      { // special case: no equality -> only one result unit
//      cout << "TU_VM_ComparePredicateValue_UReal: Single Result." << endl;
        testInst = TU_GetMidwayInstant(iv.start, iv.end);
        u1->TemporalFunction(testInst, fccr1, false);
        fccr2 = *r;
        if(unit_arg == 0)
          cmpres = fccr1.Compare( &fccr2 );
        else
          cmpres = fccr2.Compare( &fccr1 );

        compresult = ( (opcode == 0 && cmpres == 0) ||   // ==
            (opcode == 1 && cmpres != 0) ||   // #
            (opcode == 2 && cmpres  < 0) ||   // <
            (opcode == 3 && cmpres  > 0) ||   // >
            (opcode == 4 && cmpres <= 0) ||   // <=
            (opcode == 5 && cmpres >= 0)    );// >=
        newunit = UBool(iv, CcBool(true, compresult));
        localinfo->intersectionBool->StartBulkLoad();
        localinfo->intersectionBool->Add(newunit);
        localinfo->intersectionBool->EndBulkLoad();
        localinfo->NoOfResults++;
        localinfo->finished = false;
        delete eqPeriods;
        return 0;
      }
      // case: numEq > 0, at least one instant of equality
      // iterate the Periods and create result units
      // UBool::MergeAdd() will merge units with common value
      // for <= and >=
//    cout << "TU_VM_ComparePredicateValue_UReal: Multiple Results." << endl;
      localinfo->intersectionBool->StartBulkLoad();
      start = iv.start;   // the ending instant for the next interval
      lc = iv.lc;
      i = 0;              // counter for instants of equality
      eqPeriods->Get(i, actIntv);
      // handle special case: first equality in first instant
      if (start == actIntv.start)
      {
//      cout << "TU_VM_ComparePredicateValue_UReal: Handling start...";
        if (iv.lc)
        {
//        cout << " required." << endl;
          u1->TemporalFunction(u1->timeInterval.start, fccr1, false);
          fccr2 = *r;
          if(unit_arg == 0)
            cmpres = fccr1.Compare( &fccr2 );
          else
            cmpres = fccr2.Compare( &fccr1 );
          compresult = ( (opcode == 0 && cmpres == 0) ||
              (opcode == 1 && cmpres != 0) ||
              (opcode == 2 && cmpres  < 0) ||
              (opcode == 3 && cmpres  > 0) ||
              (opcode == 4 && cmpres <= 0) ||
              (opcode == 5 && cmpres >= 0)    );
          ivnew = Interval<Instant>(start, start, true, true);
          newunit = UBool(ivnew, CcBool(true, compresult));
          localinfo->intersectionBool->Add(newunit);
//        cout << "TU_VM_ComparePredicateValue_UReal: Added initial"
//             << i << endl;
          lc = false;
        } // else: equal instant not in interval!
//      else
//        cout << " not required." << endl;
        i++;
      }
      while ( i < numEq )
      {
//      cout << "TU_VM_ComparePredicateValue_UReal: Pass i=" << i << endl;
        eqPeriods->Get(i, actIntv);
//      if(actIntv.start != actIntv.end)
//      {
//        cout << "Something's wrong with actIntv!" << endl;
//      }
        end = actIntv.start;
//      cout << "  start=" << start.ToString()
//           << "  end="   << end.ToString()   << endl;
        ivnew = Interval<Instant>(start, end, lc, false);
        testInst = TU_GetMidwayInstant(start, end);
        u1->TemporalFunction(testInst, fccr1, false);
        fccr2 = *r;
        if(unit_arg == 0)
          cmpres = fccr1.Compare( &fccr2 );
        else
          cmpres = fccr2.Compare( &fccr1 );
        compresult = ( (opcode == 0 && cmpres == 0) ||
            (opcode == 1 && cmpres != 0) ||
            (opcode == 2 && cmpres  < 0) ||
            (opcode == 3 && cmpres  > 0) ||
            (opcode == 4 && cmpres <= 0) ||
            (opcode == 5 && cmpres >= 0)    );
        newunit = UBool(ivnew, CcBool(true, compresult));
        localinfo->intersectionBool->MergeAdd(newunit);
//      cout << "TU_VM_ComparePredicateValue_UReal: Added regular" << i << endl;
        if ( !(end == iv.end) || iv.rc )
        {
//        cout << "TU_VM_ComparePredicateValue_UReal: rc==true" << endl;
          ivnew = Interval<Instant>(end, end, true, true);
          compresult = (opcode == 0 || opcode == 4 || opcode == 5);
          newunit = UBool(ivnew, CcBool(true, compresult));
          localinfo->intersectionBool->MergeAdd(newunit);
        }
        start = end;
        i++;
        lc = false;
      }
      if ( start < iv.end )
      { // handle teq[numEq-1] < iv.end
        ivnew = Interval<Instant>(start, iv.end, false, iv.rc);
        testInst = TU_GetMidwayInstant(start, iv.end);
        u1->TemporalFunction(testInst, fccr1, false);
        fccr2 = *r;
        if(unit_arg == 0)
          cmpres = fccr1.Compare( &fccr2 );
        else
          cmpres = fccr2.Compare( &fccr1 );
        compresult = ( (opcode == 0 && cmpres == 0) ||
            (opcode == 1 && cmpres != 0) ||
            (opcode == 2 && cmpres  < 0) ||
            (opcode == 3 && cmpres  > 0) ||
            (opcode == 4 && cmpres <= 0) ||
            (opcode == 5 && cmpres >= 0)    );
        newunit = UBool(ivnew, CcBool(true, compresult));
        localinfo->intersectionBool->MergeAdd(newunit);
//      cout << "TU_VM_ComparePredicateValue_UReal: Added final res"
//           << i << endl;
      }
      localinfo->intersectionBool->EndBulkLoad(true);
      localinfo->NoOfResults = localinfo->intersectionBool->GetNoComponents();
      localinfo->finished = ( localinfo->NoOfResults <= 0 );
      delete eqPeriods;
      return 0;

    case REQUEST:
      if (local.addr == 0)
        return CANCEL;
      localinfo = (TUCompareValueLocalInfo*) local.addr;
      if (localinfo->finished)
        return CANCEL;
      if (localinfo->NoOfResultsDelivered >= localinfo->NoOfResults)
      {
        localinfo->finished = true;
        return CANCEL;
      }
      localinfo->intersectionBool->Get(localinfo->NoOfResultsDelivered, cu);
      result.setAddr( cu.Clone() );
      localinfo->NoOfResultsDelivered++;
      return YIELD;

    case CLOSE:
      if( local.addr != 0 )
      {
        localinfo = (TUCompareValueLocalInfo*) local.addr;
        delete localinfo->intersectionBool;
        delete localinfo;
        local.setAddr(0);
      }
  }
  return -1;
}


template<int opcode>
int TU_VM_ComparePredicateValue_URegion(Word* args, Word& result,
                                        int message, Word& local, Supplier s)
{
//   URegion   *u1  = (URegion*) args[0].addr;
//   URegion   *u2  = (URegion*) args[1].addr;

  cerr << "TU_VM_ComparePredicateValue_URegion() not yet implemented!"
       << endl;
  return -1; // should not be reached
}

/*
5.43.3 Specification for operator ~ComparePredicateValues~

*/

const string TUEqVSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>For T in {bool, int, string, real, point, region}\n"
  "(uT uT) -> (stream ubool)\n"
  "(uT  T) -> (stream ubool)\n"
  "( T uT) -> (stream ubool)</text--->"
  "<text>_ = _</text--->"
  "<text>The operator returns the value of the temporal predicate.</text--->"
  "<text>query [const ubool value ((\"2010-11-11\" "
  "\"2011-01-03\" TRUE FALSE) TRUE)] = [const ubool value "
  "((\"2011-01-01\"2012-09-17\" FALSE TRUE) TRUE)] the_mvalue</text--->"
  ") )";

const string TUNEqVSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>For T in {bool, int, string, real, point, region}\n"
  "(uT uT) -> (stream ubool)\n"
  "(uT  T) -> (stream ubool)\n"
  "( T uT) -> (stream ubool)</text--->"
  "<text>_ # _</text--->"
  "<text>The operator returns the value of the temporal predicate.</text--->"
  "<text>query [const ubool value ((\"2010-11-11\" "
  "\"2011-01-03\" TRUE FALSE) TRUE)] # [const ubool value "
  "((\"2011-01-01\" \"2012-09-17\" FALSE TRUE) TRUE)] the_mvalue</text--->"
  ") )";

const string TULtVSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>For T in {bool, int, string, real}\n"
  "(uT uT) -> (stream ubool)\n"
  "(uT  T) -> (stream ubool)\n"
  "( T uT) -> (stream ubool)</text--->"
  "<text>_ < _</text--->"
  "<text>The operator returns the value of the temporal predicate.</text--->"
  "<text>query [const ubool value ((\"2010-11-11\" "
  "\"2011-01-03\" TRUE FALSE) TRUE)] < [const ubool value "
  "((\"2011-01-01\" \"2012-09-17\" FALSE TRUE) TRUE)] the_mvalue</text--->"
  ") )";

const string TUBtVSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>For T in {bool, int, string, real}\n"
  "(uT uT) -> (stream ubool)\n"
  "(uT  T) -> (stream ubool)\n"
  "( T uT) -> (stream ubool)</text--->"
  "<text>_ > _</text--->"
  "<text>The operator returns the value of the temporal predicate.</text--->"
  "<text>query [const ubool value ((\"2010-11-11\" "
  "\"2011-01-03\" TRUE FALSE) TRUE)] > [const ubool value "
  "((\"2011-01-01\" \"2012-09-17\" FALSE TRUE) TRUE)] the_mvalue</text--->"
  ") )";

const string TULtEqVSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>For T in {bool, int, string, real}\n"
  "(uT uT) -> (stream ubool)\n"
  "(uT  T) -> (stream ubool)\n"
  "( T uT) -> (stream ubool)</text--->"
  "<text>_ <= _</text--->"
  "<text>The operator returns the value of the temporal predicate.</text--->"
  "<text>query [const ubool value ((\"2010-11-11\" "
  "\"2011-01-03\" TRUE FALSE) TRUE)] <= [const ubool value "
  "((\"2011-01-01\" \"2012-09-17\" FALSE TRUE) TRUE)] the_mvalue</text--->"
  ") )";

const string TUBtEqVSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>For T in {bool, int, string, real}\n"
  "(uT uT) -> (stream ubool)\n"
  "(uT  T) -> (stream ubool)\n"
  "( T uT) -> (stream ubool)</text--->"
  "<text>_ >= _</text--->"
  "<text>The operator returns the value of the temporal predicate.</text--->"
  "<text>query [const ubool value ((\"2010-11-11\" "
  "\"2011-01-03\" TRUE FALSE) TRUE)] >= [const ubool value "
  "((\"2011-01-01\" \"2012-09-17\" FALSE TRUE) TRUE)] the_mvalue</text--->"
  ") )";

/*
5.43.4 Selection Function of operator ~ComparePredicateValues~

*/
template<int opcode>
int TU_Select_ComparePredicateValue ( ListExpr args )
{
  string argstr;
  nl->WriteToString(argstr, args);

  if (argstr == "(ubool ubool)")
    return 0 + opcode;
  if (argstr == "(uint uint)")
    return 6 + opcode;
  if (argstr == "(ustring ustring)")
    return 12 + opcode;
  if (argstr == "(ureal ureal)")
    return 18 + opcode;
  if (argstr == "(upoint upoint)")
    return 24 + opcode;
  if (argstr == "(uregion uregion)")
    return 26 + opcode;

  if (argstr == "(bool ubool)")
     return 28 + opcode;
  if (argstr == "(ubool bool)")
     return 34 + opcode;
  if (argstr == "(int uint)")
     return 40 + opcode;
  if (argstr == "(unit int)")
     return 46 + opcode;
  if (argstr == "(string ustring)")
     return 52 + opcode;
  if (argstr == "(ustring string)")
     return 58 + opcode;
  if (argstr == "(real ureal)")
     return 64 + opcode;
  if (argstr == "(ureal real)")
     return 70 + opcode;
  if (argstr == "(point upoint)")
     return 76 + opcode;
  if (argstr == "(upoint point)")
     return 78 + opcode;

  return -1; // should not be reached!
}

ValueMapping TU_VMMap_ComparePredicateValue[] =
  {
    TU_VM_ComparePredicateValue_Const<CcBool,0>,    //  0
    TU_VM_ComparePredicateValue_Const<CcBool,1>,
    TU_VM_ComparePredicateValue_Const<CcBool,2>,
    TU_VM_ComparePredicateValue_Const<CcBool,3>,
    TU_VM_ComparePredicateValue_Const<CcBool,4>,
    TU_VM_ComparePredicateValue_Const<CcBool,5>,    //  5
    TU_VM_ComparePredicateValue_Const<CcInt,0>,     //  6
    TU_VM_ComparePredicateValue_Const<CcInt,1>,
    TU_VM_ComparePredicateValue_Const<CcInt,2>,
    TU_VM_ComparePredicateValue_Const<CcInt,3>,
    TU_VM_ComparePredicateValue_Const<CcInt,4>,
    TU_VM_ComparePredicateValue_Const<CcInt,5>,    //  11
    TU_VM_ComparePredicateValue_Const<CcString,0>, //  12
    TU_VM_ComparePredicateValue_Const<CcString,1>,
    TU_VM_ComparePredicateValue_Const<CcString,2>,
    TU_VM_ComparePredicateValue_Const<CcString,3>,
    TU_VM_ComparePredicateValue_Const<CcString,4>,
    TU_VM_ComparePredicateValue_Const<CcString,5>, // 17
    TU_VM_ComparePredicateValue_UReal<0>,          // 18
    TU_VM_ComparePredicateValue_UReal<1>,
    TU_VM_ComparePredicateValue_UReal<2>,
    TU_VM_ComparePredicateValue_UReal<3>,
    TU_VM_ComparePredicateValue_UReal<4>,
    TU_VM_ComparePredicateValue_UReal<5>,          // 23
    TU_VM_ComparePredicateValue_UPoint<0>,         // 24
    TU_VM_ComparePredicateValue_UPoint<1>,         // 25
    TU_VM_ComparePredicateValue_URegion<0>,        // 26
    TU_VM_ComparePredicateValue_URegion<1>,        // 27

    TU_VM_ComparePredicateValue_Const_T<CcBool,0,1>,// 28
    TU_VM_ComparePredicateValue_Const_T<CcBool,1,1>,
    TU_VM_ComparePredicateValue_Const_T<CcBool,2,1>,
    TU_VM_ComparePredicateValue_Const_T<CcBool,3,1>,
    TU_VM_ComparePredicateValue_Const_T<CcBool,4,1>,
    TU_VM_ComparePredicateValue_Const_T<CcBool,5,1>,
    TU_VM_ComparePredicateValue_Const_T<CcBool,0,0>,//34
    TU_VM_ComparePredicateValue_Const_T<CcBool,1,0>,
    TU_VM_ComparePredicateValue_Const_T<CcBool,2,0>,
    TU_VM_ComparePredicateValue_Const_T<CcBool,3,0>,
    TU_VM_ComparePredicateValue_Const_T<CcBool,4,0>,
    TU_VM_ComparePredicateValue_Const_T<CcBool,5,0>,

    TU_VM_ComparePredicateValue_Const_T<CcInt,0,1>,// 40
    TU_VM_ComparePredicateValue_Const_T<CcInt,1,1>,
    TU_VM_ComparePredicateValue_Const_T<CcInt,2,1>,
    TU_VM_ComparePredicateValue_Const_T<CcInt,3,1>,
    TU_VM_ComparePredicateValue_Const_T<CcInt,4,1>,
    TU_VM_ComparePredicateValue_Const_T<CcInt,5,1>,
    TU_VM_ComparePredicateValue_Const_T<CcInt,0,0>,// 46
    TU_VM_ComparePredicateValue_Const_T<CcInt,1,0>,
    TU_VM_ComparePredicateValue_Const_T<CcInt,2,0>,
    TU_VM_ComparePredicateValue_Const_T<CcInt,3,0>,
    TU_VM_ComparePredicateValue_Const_T<CcInt,4,0>,
    TU_VM_ComparePredicateValue_Const_T<CcInt,5,0>,

    TU_VM_ComparePredicateValue_Const_T<CcString,0,1>,// 52
    TU_VM_ComparePredicateValue_Const_T<CcString,1,1>,
    TU_VM_ComparePredicateValue_Const_T<CcString,2,1>,
    TU_VM_ComparePredicateValue_Const_T<CcString,3,1>,
    TU_VM_ComparePredicateValue_Const_T<CcString,4,1>,
    TU_VM_ComparePredicateValue_Const_T<CcString,5,1>,
    TU_VM_ComparePredicateValue_Const_T<CcString,0,0>,// 58
    TU_VM_ComparePredicateValue_Const_T<CcString,1,0>,
    TU_VM_ComparePredicateValue_Const_T<CcString,2,0>,
    TU_VM_ComparePredicateValue_Const_T<CcString,3,0>,
    TU_VM_ComparePredicateValue_Const_T<CcString,4,0>,
    TU_VM_ComparePredicateValue_Const_T<CcString,5,0>,

    TU_VM_ComparePredicateValue_UReal_CcReal<0,1>,    // 64
    TU_VM_ComparePredicateValue_UReal_CcReal<1,1>,
    TU_VM_ComparePredicateValue_UReal_CcReal<2,1>,
    TU_VM_ComparePredicateValue_UReal_CcReal<3,1>,
    TU_VM_ComparePredicateValue_UReal_CcReal<4,1>,
    TU_VM_ComparePredicateValue_UReal_CcReal<5,1>,
    TU_VM_ComparePredicateValue_UReal_CcReal<0,0>,    // 70
    TU_VM_ComparePredicateValue_UReal_CcReal<1,0>,
    TU_VM_ComparePredicateValue_UReal_CcReal<2,0>,
    TU_VM_ComparePredicateValue_UReal_CcReal<3,0>,
    TU_VM_ComparePredicateValue_UReal_CcReal<4,0>,
    TU_VM_ComparePredicateValue_UReal_CcReal<5,0>,    // 75

    TU_VM_ComparePredicateValue_UPoint_Point<0,1>,    // 76
    TU_VM_ComparePredicateValue_UPoint_Point<1,1>,    // 77
    TU_VM_ComparePredicateValue_UPoint_Point<0,0>,    // 78
    TU_VM_ComparePredicateValue_UPoint_Point<1,0>     // 79
  };

/*
5.43.5 Definition of operator ~ComparePredicateValues~

*/
Operator temporalunitvalisequal
(
 "=",
 TUEqVSpec,
 80,
 TU_VMMap_ComparePredicateValue,
 TU_Select_ComparePredicateValue<0>,
 TUCompareValueEqPredicatesTypeMap
 );

Operator temporalunitvalisnotequal
(
 "#",
 TUNEqVSpec,
 80,
 TU_VMMap_ComparePredicateValue,
 TU_Select_ComparePredicateValue<1>,
 TUCompareValueEqPredicatesTypeMap
 );

Operator temporalunitvalsmaller
(
 "<",
 TULtVSpec,
 80,
 TU_VMMap_ComparePredicateValue,
 TU_Select_ComparePredicateValue<2>,
 TUCompareValuePredicatesTypeMap
 );

Operator temporalunitvalbigger
(
 ">",
 TUBtVSpec,
 80,
 TU_VMMap_ComparePredicateValue,
 TU_Select_ComparePredicateValue<3>,
 TUCompareValuePredicatesTypeMap
 );

Operator temporalunitvalsmallereq
(
 "<=",
 TULtEqVSpec,
 80,
 TU_VMMap_ComparePredicateValue,
 TU_Select_ComparePredicateValue<4>,
 TUCompareValuePredicatesTypeMap
 );

Operator temporalunitvalbiggereq
(
 ">=",
 TUBtEqVSpec,
 80,
 TU_VMMap_ComparePredicateValue,
 TU_Select_ComparePredicateValue<5>,
 TUCompareValuePredicatesTypeMap
 );


/*
5.44 Operator ~length~

Calculate the spatial length of the movement.

----
     length: upoint --> real

----

*/

/*
5.44.1 Type mapping function for ~length~

---- upoint [ x geoid ] --> real
----

*/
ListExpr TUTypeMapLength( ListExpr args )
{
  string errmsg = "Expected (upoint) or (upoint x geoid).";
  int noargs = nl->ListLength(args);
  if((noargs<1) || (noargs>2)){
    return listutils::typeError(errmsg);
  }
  if(!listutils::isSymbol(nl->First(args),UPoint::BasicType())){
    return listutils::typeError(errmsg);
  }
  if(    (noargs==2)
    && (!listutils::isSymbol(nl->Second(args),Geoid::BasicType())) ){
    return listutils::typeError(errmsg);
  }
  return nl->SymbolAtom(CcReal::BasicType());
}

/*
5.44.2 Value mapping for operator ~length~

*/
int TUUnitLength(Word* args,Word& result,int message,Word& local,Supplier s)
{
  result = qp->ResultStorage( s );
  CcReal  *res   = (CcReal*)result.addr;
  UPoint *input = (UPoint*)args[0].addr;

  if(qp->GetNoSons(s)==2){ // variant using (LON,LAT)-coordinates
    Geoid* g = static_cast<Geoid*>(args[1].addr);
    if(!g->IsDefined()){
      res->Set(false, 0.0);
      return 0;
    }
    input->Length(*g, *res);
  } else { // normal variant using (X,Y)-coordinates
    input->Length( *res );
  }
  return 0;
}

/*
5.44.3 Specification for operator ~length~

*/
const string TULengthSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text> upoint [ x geoid ] -> real</text--->"
    "<text> length( Up [, Geoid ] )</text--->"
    "<text>The operator returns the distance of the unit's initial and final"
    "position. If the optional parameter Geoid is not used, spatial "
    "coordinates are interpreted as metric (X,Y)-pairs, otherwise as "
    "geographic (LON,LAT)-coordinates relative to the passed geoid.</text--->"
    "<text>query units(Trains feed extract[Trip]) use[fun(U:upoint) "
    "length(U)] transformstream sum[elem]</text--->"
    ") )";

/*
5.44.4 Selection Function of operator ~length~

none - uses simpleselect
*/

/*
5.44.5 Definition of operator ~length~

*/

Operator temporalunitlength( "length",
                             TULengthSpec,
                             TUUnitLength,
                             Operator::SimpleSelect,
                             TUTypeMapLength);

/*
5.45 Operator ~canmeet~

The predicate predictes whether two mpoint objects will become close to
eachother (in terms of a given distance threshold), within a given time
duration assuming that they keep the speed and direction of the given two
upoints.

*/

/*
5.45.1 Type mapping function for ~canmeet~

*/

ListExpr
TypeMapTemporalUnitCanMeet( ListExpr args )
{
  ListExpr upoint1, upoint2, distance, duration;
  string outstr;

  if ( nl->IsAtom( args ) || nl->ListLength( args ) != 4 )
    {
      nl->WriteToString(outstr, args);
      ErrorReporter::ReportError("Operator canmeet expects a list of "
                                 "length four, but gets '" + outstr +
                                 "'.");
      return nl->SymbolAtom( Symbol::TYPEERROR() );
    }

  upoint1 = nl->First(args);
  upoint2 = nl->Second(args);
  distance = nl->Third(args);
  duration = nl->Fourth(args);

  // check for compatibility of arguments
  if( !nl->IsAtom(upoint1) || !nl->IsEqual(upoint1, UPoint::BasicType()))
    {
      nl->WriteToString(outstr, upoint1);
      ErrorReporter::ReportError("Operator canmeet expects upoint as a first "
                                 "argument, but gets '" + outstr + "'.");
      return nl->SymbolAtom( Symbol::TYPEERROR() );
    }

  if( !nl->IsAtom(upoint2) || !nl->IsEqual(upoint2, UPoint::BasicType()))
     {
       nl->WriteToString(outstr, upoint2);
       ErrorReporter::ReportError("Operator canmeet expects upoint as a second "
                                  "argument, but gets '" + outstr + "'.");
       return nl->SymbolAtom( Symbol::TYPEERROR() );
     }

  if( !nl->IsAtom(distance) || !nl->IsEqual(distance, CcReal::BasicType()))
     {
       nl->WriteToString(outstr, distance);
       ErrorReporter::ReportError("Operator canmeet expects real as a third "
                                  "argument, but gets '" + outstr + "'.");
       return nl->SymbolAtom( Symbol::TYPEERROR() );
     }

  if( !nl->IsAtom(duration) || !nl->IsEqual(duration, Duration::BasicType()))
     {
       nl->WriteToString(outstr, duration);
       ErrorReporter::ReportError("Operator canmeet expects duration as a "
                                  "fourth "
                                  "argument, but gets '" + outstr + "'.");
       return nl->SymbolAtom( Symbol::TYPEERROR() );
     }
  return nl->SymbolAtom( CcBool::BasicType() );
}

/*
5.45.2 Value mapping for operator ~canmeet~

*/

int TUCanMeet( Word* args, Word& result, int message,
                              Word& local, Supplier s )
{
  bool debugme=false;
  Interval<Instant> iv;

  //  Word a1, a2;
  UPoint *u1, *u2;
  CcReal* distThreshold;
  Instant* timeThreshold;
  result = qp->ResultStorage( s );
  CcBool* res = (CcBool*) result.addr;

  u1 = (UPoint*)(args[0].addr);
  u2 = (UPoint*)(args[1].addr);
  distThreshold = (CcReal*)(args[2].addr);
  timeThreshold = (Instant*)(args[3].addr);
  double tThresholdMin=0, tThresholdMax=0;
  if (!u1->IsDefined() ||
      !u2->IsDefined() ||
      !distThreshold->IsDefined() ||
      ! timeThreshold->IsDefined())
    { // return undefined ureal
      res->SetDefined( false );
    }
  else
    {
      // calculate result
      res->SetDefined( true );
      // 1- Extend the two upoints so that their time intervals are equal
      UPoint u1ex(*u1), u2ex(*u2);
      Point newPoint;
      if(u1->timeInterval.start < u2->timeInterval.start)
        //extend u2 backward
      {
        tThresholdMin = u2->timeInterval.start.ToDouble();
        u2->TemporalFunction(u1->timeInterval.start, newPoint, true);
        u2ex.timeInterval.start = u1->timeInterval.start;
        u2ex.p0 = newPoint;
      }
      else
        //extend u1 backward
      {
        tThresholdMin = u1->timeInterval.start.ToDouble();
        u1->TemporalFunction(u2->timeInterval.start, newPoint, true);
        u1ex.timeInterval.start = u2->timeInterval.start;
        u1ex.p0 = newPoint;
      }

      if(u1->timeInterval.end < u2->timeInterval.end)
        //extend u1 forward
      {
        tThresholdMax = u2->timeInterval.end.ToDouble() +
          timeThreshold->ToDouble();
        u1->TemporalFunction(u2->timeInterval.end, newPoint, true);
        u1ex.timeInterval.end = u2->timeInterval.end;
        u1ex.p1 = newPoint;
      }
      else
        //extend u2 forward
      {
        tThresholdMax = u1->timeInterval.end.ToDouble() +
                  timeThreshold->ToDouble();
        u2->TemporalFunction(u1->timeInterval.end, newPoint, true);
        u2ex.timeInterval.end = u1->timeInterval.end;
        u2ex.p1 = newPoint;
      }

      // 2- Compute the distance between the two extended units
      UReal dist(0);
      u1ex.Distance( u2ex, dist );
      if(debugme)
        dist.Print(cerr);
      // 3- Compute the time when the distance reaches the distThreshould
      double c= dist.c - ((dist.r)?
          distThreshold->GetRealval() * distThreshold->GetRealval():
            distThreshold->GetRealval());
      double coeff = (dist.b * dist.b) - (4 * dist.a * c);
      if(coeff < 0)
      {
        res->Set(true, false);
        return 0;
      }
      coeff = sqrt(coeff);
      double t1= ((dist.b * -1) + coeff )/ (2 * dist.a);
      double t2= ((dist.b * -1) - coeff )/ (2 * dist.a);
      double intervalStart= u1ex.timeInterval.start.ToDouble();
      if( (t1 + intervalStart) > tThresholdMin &&
          (t1 + intervalStart) < tThresholdMax)
        res->Set(true, true);
      else if( (t2 + intervalStart) > tThresholdMin &&
          (t2 + intervalStart) < tThresholdMax)
        res->Set(true, true);
      else
        res->Set(true, false);
    }
  // pass on result
  return 0;
}


/*
5.45.3 Specification for operator ~canmeet~

*/
const string TemporalSpecCanMeet =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "(<text>"
  "( upoint upoint real duration) -> bool</text--->"
  "<text>canmeet( _, _, _, _)</text--->"
  "<text> The predicate predictes whether two upoint objects will become close"
  " to eachother (in terms of the given distance threshold), within the given "
  " time duration, assuming that they keep their speed and direction.</text--->"
  "<text>canmeet(upoint1,upoint2, 50.0, now() + create_duration(0, 5000))"
  "</text--->"
  ") )";

/*
5.45.5 Definition of operator ~canmeet~

*/

Operator temporalunitcanmeet( "canmeet",
                               TemporalSpecCanMeet,
                               TUCanMeet,
                               Operator::SimpleSelect,
                               TypeMapTemporalUnitCanMeet);



/*
5.46 Operator ~when~

5.46.1 Type Mapping for ~when~

*/
ListExpr
UnitWhenTypeMap( ListExpr args )
{
  ListExpr arg1, arg2;
  string argstr;

  if ( nl->ListLength( args ) != 2 )
    {
      ErrorReporter::ReportError("Operator when expects a list of length 2.");
      return nl->SymbolAtom( Symbol::TYPEERROR() );
    }

  arg1 = nl->First( args );
  arg2 = nl->Second( args );

  nl->WriteToString(argstr, arg2);
  if ( !( nl->IsEqual( arg2, MBool::BasicType() ) ) )
    {
      ErrorReporter::ReportError("Operator when expects a second argument"
            " of type " + MBool::BasicType() + " but gets '" + argstr + "'.");
      return nl->SymbolAtom( Symbol::TYPEERROR() );
    }

  if( nl->IsAtom( arg1 ) )
    {
      if( nl->IsEqual( arg1, UBool::BasicType() ) )
        return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
               nl->SymbolAtom(UBool::BasicType()));
      if( nl->IsEqual( arg1, UInt::BasicType() ) )
        return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
               nl->SymbolAtom(UInt::BasicType()));
      if( nl->IsEqual( arg1, UReal::BasicType() ) )
        return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
               nl->SymbolAtom(UReal::BasicType()));
      if( nl->IsEqual( arg1, UPoint::BasicType() ) )
        return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                               nl->SymbolAtom(UPoint::BasicType()));
      if( nl->IsEqual( arg1, UString::BasicType() ) )
        return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
               nl->SymbolAtom(UString::BasicType()));

      nl->WriteToString(argstr, arg1);
      ErrorReporter::ReportError("Operator when expects a first argument "
                                 "of type T in {ubool, uint, ureal, upoint, "
                                 "ustring, uregion} but gets a '"
                                 + argstr + "'.");
      return nl->SymbolAtom( Symbol::TYPEERROR() );
    }

  if( !( nl->IsAtom( arg1 ) ) && ( nl->ListLength( arg1 ) == 2) )
    {
      nl->WriteToString(argstr, arg1);
      if ( !( TypeOfRelAlgSymbol(nl->First(arg1)) == stream ) )
        {
          ErrorReporter::ReportError("Operator when expects as first "
                                     "argument a list with structure 'T' or "
                                     "'stream(T)', T in {ubool, uint, ureal, "
                                     "upoint, ustring, ureagion} but gets a "
                                     "list with structure '" + argstr + "'.");
          return nl->SymbolAtom( Symbol::TYPEERROR() );
        }

      if( nl->IsEqual( nl->Second(arg1), UBool::BasicType() ) )
        return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                               nl->SymbolAtom(UBool::BasicType()));
      if( nl->IsEqual( nl->Second(arg1), UInt::BasicType() ) )
        return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                               nl->SymbolAtom(UInt::BasicType()));
      if( nl->IsEqual( nl->Second(arg1), UReal::BasicType() ) )
         return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                                nl->SymbolAtom(UReal::BasicType()));
      if( nl->IsEqual( nl->Second(arg1), UPoint::BasicType() ) )
         return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                                 nl->SymbolAtom(UPoint::BasicType()));
      if( nl->IsEqual( nl->Second(arg1), UString::BasicType() ) )
         return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                                 nl->SymbolAtom(UString::BasicType()));

      nl->WriteToString(argstr, nl->Second(arg1));
      ErrorReporter::ReportError("Operator when expects a type "
                              "(stream T); T in {ubool, uint, ureal, upoint, "
                              "ustring, uregion} but gets '(stream "
                              + argstr + ")'.");
      return nl->SymbolAtom( Symbol::TYPEERROR() );
    };

  nl->WriteToString( argstr, args );
  ErrorReporter::ReportError("Operator when encountered an "
                             "unmatched typerror for arguments '"
                             + argstr + "'.");
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

/*
5.46.2 Value Mapping for ~atperiods~

*/
struct WhenLocalInfo
{
  Word uWord;     // the address of the unit value
  Word pWord;    //  the adress of the periods value
  int  j;       //   save the number of the interval
};

/*
Variant 1: first argument is a scalar value

*/

template <class Alpha>
int MappingUnitWhen( Word* args, Word& result, int message,
                          Word& local, Supplier s )
{
  AtPeriodsLocalInfo *localinfo;
  Interval<Instant> interval;
  Alpha *unit;
  Alpha r(true);
  Periods* periods;

  switch( message )
  {
  case OPEN:
  {
// #ifdef TUA_DEBUG
//     cout << "\nMappingUnitAtPeriods: OPEN" << endl;
// #endif
    Periods* p= new Periods(0);
    MBool* mb= static_cast<MBool*>(args[1].addr);
    CcBool tru(true, true);
    MBool mbTrue(0);
    mb->At(tru, mbTrue);
    mbTrue.DefTime(*p);

    localinfo = new AtPeriodsLocalInfo;
    localinfo->uWord = args[0];
    localinfo->pWord = SetWord(p);
    localinfo->j = 0;
    local.setAddr(localinfo);
    return 0;
  }break;
  case REQUEST:
  {
// #ifdef TUA_DEBUG
//     cout << "\nMappingUnitAtPeriods: REQUEST" << endl;
// #endif
    if( local.addr == 0 )
      return CANCEL;
    localinfo = (AtPeriodsLocalInfo *)local.addr;
    unit      =   (Alpha*)localinfo->uWord.addr;
    periods   = (Periods*)localinfo->pWord.addr;

    if( !unit->IsDefined()    ||
        !periods->IsDefined() ||
        periods->IsEmpty()       )
    {
      result.setAddr(0);
      return CANCEL;
    }
// #ifdef TUA_DEBUG
//     cout << "   Unit's timeInterval u="
//          << TUPrintTimeInterval( unit->timeInterval ) << endl;
// #endif
    if( localinfo->j >= periods->GetNoComponents() )
      {
        result.setAddr(0);
// #ifdef TUA_DEBUG
//           cout << "Maquery train7 inside train7sectionsppingUnitAtPeriods: "
//                << "REQUEST finished: CANCEL (1)"
//                << endl;
// #endif
        return CANCEL;
      }
    periods->Get( localinfo->j, interval );
    localinfo->j++;
// #ifdef TUA_DEBUG
//     cout << "   Probing timeInterval p ="
//          << TUPrintTimeInterval(interval)
//          << endl;
// #endif
    while( interval.Before( unit->timeInterval ) &&
           localinfo->j < periods->GetNoComponents() )
    { // forward to first candidate interval
        periods->Get(localinfo->j, interval);
        localinfo->j++;
// #ifdef TUA_DEBUG
//         cout << "   Probing timeInterval="
//             << TUPrintTimeInterval(interval)
//             << endl;
//         if (interval.Before( unit->timeInterval ))
//           cout << "     p is before u" << endl;
//         if (localinfo->j < periods->GetNoComponents())
//           cout << "   j < #Intervals" << endl;
// #endif
    }

    if( unit->timeInterval.Before( interval ) )
      { // interval after unit-deftime --> finished
        result.addr = 0;
// #ifdef TUA_DEBUG
//           cout << "MappingUnitAtPeriods: REQUEST finished: CANCEL (2)"
//                << endl;
// #endif
        return CANCEL;
    }

    if(unit->timeInterval.Intersects( interval ))
    { // interval intersectd unit's deftime --> produce result
        // create unit restricted to interval
        unit->AtInterval( interval, r );
        Alpha* aux = new Alpha( r );
        result.setAddr( aux );
// #ifdef TUA_DEBUG
//             cout << "   Result interval="
//                  << TUPrintTimeInterval(aux->timeInterval)
//                  << endl;
//             cout << "   Result defined=" << aux->IsDefined()
//                  << endl;
//             cout << "MappingUnitAtPeriods: REQUEST finished: YIELD"
//                  << endl;
// #endif
        return YIELD;
    }

    if( localinfo->j >= periods->GetNoComponents() )
    { // Passed last interval --> finished
      result.addr = 0;
// #ifdef TUA_DEBUG
//       cout << "MappingUnitAtPeriods: REQUEST finished: CANCEL (3)"
//         << endl;
// #endif
      return CANCEL;
    }

    result.setAddr(0 );
    cout << "MappingUnitWhen: REQUEST finished: CANCEL (4)"
         << endl;
    cout << "Intervals should overlap: " << endl;
    cout << "  Unit's timeInterval = ";
    TUPrintTimeInterval(unit->timeInterval);
    cout << "  Current Period's interval = ";
    TUPrintTimeInterval(interval);
    cout << endl;
    assert( false );
    return CANCEL; // should not happen
  }break;
  case CLOSE:
  {
    if( local.addr != 0 )
    {
      AtPeriodsLocalInfo *li= static_cast<AtPeriodsLocalInfo *>(local.addr);
      Periods* p = static_cast<Periods*>(li->pWord.addr);
      delete p;
      delete li;
      local.setAddr(Address(0));
    }
    return 0;
  }
  }
  // should not happen:
  return -1;
}

/*
Variant 2: first argument is a stream

*/

struct WhenLocalInfoUS
{
  Word uWord;  // address of the input stream
  Word pWord;  // address of the input periods value
  int j;       // interval counter for within periods
};


template <class Alpha>
int MappingUnitStreamWhen( Word* args, Word& result, int message,
                                Word& local, Supplier s )
{
  AtPeriodsLocalInfoUS *localinfo;
  Alpha *unit, *aux;
  Alpha resultUnit(true);
  Periods *periods;
  Interval<Instant> interval;
  bool foundUnit = false;

  switch( message )
  {
  case OPEN:
  {
    Periods* p= new Periods(0);
    MBool* mb= static_cast<MBool*>(args[1].addr);
    CcBool tru(true, true);
    MBool mbTrue(0);
    mb->At(tru, mbTrue);
    mbTrue.DefTime(*p);

    localinfo = new AtPeriodsLocalInfoUS;
    localinfo->pWord = p;
    localinfo->j = 0;                              // init interval counter
    qp->Open( args[0].addr );                      // open stream of units
    qp->Request( args[0].addr, localinfo->uWord ); // request first unit
    if ( !( qp->Received( args[0].addr) ) ){
      localinfo->uWord.addr = 0;
      result.addr = 0;
      return CANCEL;
    }
    local.setAddr(localinfo);                    // pass up link to localinfo
    return 0;
  }break;
  case REQUEST:
  {
      if ( local.addr == 0 )
        return CANCEL;
      localinfo = (AtPeriodsLocalInfoUS *) local.addr; // restore local data
      if ( localinfo->uWord.addr == 0 ) { result.addr = 0; return CANCEL; }
      unit = (Alpha *) localinfo->uWord.addr;
      if ( localinfo->pWord.addr == 0 ) { result.addr = 0; return CANCEL; }
      periods = (Periods *) localinfo->pWord.addr;

      if( !periods->IsDefined() || periods->IsEmpty()       )
        return CANCEL;

      // search for a pair of overlapping unit/interval:
      while (1){
        if ( localinfo->j == periods->GetNoComponents() ){// redo first interval
          localinfo->j = 0;
          unit->DeleteIfAllowed();                // delete original unit?
          localinfo->uWord.addr = 0;
          foundUnit = false;
          while(!foundUnit){
            qp->Request(args[0].addr, localinfo->uWord);  // get new unit
            if( qp->Received( args[0].addr ) )
              unit = (Alpha *) localinfo->uWord.addr;
            else {
              localinfo->uWord.addr = 0;
              result.addr = 0;
              return CANCEL;
            }   // end of unit stream
            foundUnit = unit->IsDefined();
          }
        }
        periods->Get(localinfo->j, interval);       // get an interval
        if (    !( interval.Before( unit->timeInterval ) )
                  && !( unit->timeInterval.Before( interval) ) )
          break;                           // found candidate, break while
        localinfo->j++;                             // next interval, loop
      }

      // We have an interval overlapping the unit's interval now
      // Return unit restricted to overlapping part of both intervals
      if (!unit->timeInterval.Intersects( interval) ){ // This may not happen!
        cout << __FILE__ << __LINE__ << __PRETTY_FUNCTION__
             << ": Intervals do not overlap, but should do so:" << endl;
        cout << "  Unit's timeInterval = ";
        TUPrintTimeInterval(unit->timeInterval);
        cout << endl << "  Current Period's interval = ";
        TUPrintTimeInterval(interval);
        cout << endl;
        assert(false);
      }
      unit->AtInterval( interval, resultUnit); // intersect unit and interval
      aux = new Alpha( resultUnit );
      result.setAddr( aux );
      localinfo->j++;                           // increase interval counter
      return YIELD;
  }break;
  case CLOSE:
  {
    if ( local.addr != 0 )
      {
        qp->Close( args[0].addr );
        localinfo = (AtPeriodsLocalInfoUS *) local.addr;
        if ( localinfo->uWord.addr != 0 )
        {
          unit = (Alpha *) localinfo->uWord.addr;
          unit->DeleteIfAllowed();   // delete remaining original unit
        }
        Periods* p = static_cast<Periods*>(localinfo->pWord.addr);
        delete p;
        delete (AtPeriodsLocalInfoUS *)localinfo;
        local.setAddr(Address(0));
      }
    return 0;
  }

  } // end switch

  return -1; // should never be reached

} // end MappingUnitStreamAtPeriods

/*
5.46.3 Specification for operator ~atperiods~

*/
const string
TemporalSpecWhen  =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\") "
"( <text>For T in {int, bool, real, string, point, region}:\n"
"(uT periods) -> stream uT\n"
"((stream uT) periods) -> stream uT</text--->"
"<text>_ when[_] </text--->"
"<text>restrict the movement to the times on which the given mbool is true."
"</text--->"
"<text>units(mpoint1) when[speed(mpoint1) > 20.0] </text--->) )";

/*
5.46.4 Value map of operator ~when~

*/

ValueMapping temporalunitwhenmap[] =
  { MappingUnitWhen<UBool>,
    MappingUnitWhen<UInt>,
    MappingUnitWhen<UReal>,
    MappingUnitWhen<UPoint>,
    MappingUnitWhen<UString>,
    MappingUnitStreamWhen<UBool>,
    MappingUnitStreamWhen<UInt>,
    MappingUnitStreamWhen<UReal>,
    MappingUnitStreamWhen<UPoint>,
    MappingUnitStreamWhen<UString>
  };

/*
5.46.4 Selection Function of operator ~when~

*/

int
WhenSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );

  if (nl->IsAtom( arg1 ) )
    {
      if( nl->SymbolValue( arg1 ) == UBool::BasicType() )
        return 0;
      if( nl->SymbolValue( arg1 ) == UInt::BasicType() )
        return 1;
      if( nl->SymbolValue( arg1 ) == UReal::BasicType() )
        return 2;
      if( nl->SymbolValue( arg1 ) == UPoint::BasicType() )
        return 3;
      if( nl->SymbolValue( arg1 ) == UString::BasicType() )
        return 4;
    }

  if(   !( nl->IsAtom( arg1 ) )
      && ( nl->ListLength(arg1) == 2 )
      && ( TypeOfRelAlgSymbol(nl->First(arg1)) == stream ) )
    { if( nl->IsEqual( nl->Second(arg1), UBool::BasicType() ) )
        return 5;
      if( nl->IsEqual( nl->Second(arg1), UInt::BasicType() ) )
        return 6;
      if( nl->IsEqual( nl->Second(arg1), UReal::BasicType() ) )
        return 7;
      if( nl->IsEqual( nl->Second(arg1), UPoint::BasicType() ) )
        return 8;
      if( nl->IsEqual( nl->Second(arg1), UString::BasicType() ) )
        return 9;
    }

  return -1; // This point should never be reached
}

/*
5.46.6  Definition of operator ~when~

*/
Operator temporalunitwhen( "when",
                            TemporalSpecWhen,
                            10,
                            temporalunitwhenmap,
                            WhenSelect,
                            UnitWhenTypeMap );


/*
5.47.1 Operator atRect Value Mapping

*/

int atRectUVM( Word* args, Word& result, int message, Word&
 local, Supplier s ){
  UPoint* up = (UPoint*) args[0].addr;
  Rectangle<2>* rect = (Rectangle<2>*) args[1].addr;
  result = qp->ResultStorage(s);
  UPoint* res = (UPoint*) result.addr;
  up->At(*rect,*res);
  return 0;
}

/*
5.47.2 Operator atRect Type Mapping

   Signature is: upoint x rect -> upoint

*/

ListExpr atRectUTM(ListExpr args){
  string err ="upoint x rect expected";
  if(!nl->HasLength(args,2)){
    return listutils::typeError(err + " (wrong number of arguments)");
  }
  if(!UPoint::checkType(nl->First(args)) ||
     !Rectangle<2>::checkType(nl->Second(args))){
    return listutils::typeError(err);
  }
  return nl->SymbolAtom(UPoint::BasicType());
}

/*
5.47.3 Specification

*/
const string atRectUSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>upoint x rect -> upoint </text---> "
    "<text> up atRect r  </text--->"
    "<text>Restricts the upoint up to the part inside r "
    "</text--->"
    "<text>query getunit(train7, 1) atRect bbox(thecenter)</text--->"
    ") )";

/*
5.47.4 Definition of operator atRect

*/
Operator atRectU( "atRect",
                 atRectUSpec,
                 atRectUVM,
                 Operator::SimpleSelect,
                 atRectUTM);


/*
5.47 Operator ~contains~

----
     Secinterval x SecInterval -> bool
     Secinterval x Periods -> bool
     Periods x SecInterval -> bool
     Periods x Periods -> bool
----

*/

/*
5.47.1 Class ~SecInterval~

*/
const string SecInterval::BasicType() {
  return "interval";
}

const bool SecInterval::checkType(const ListExpr type) {
  return listutils::isSymbol(type, BasicType());
}

bool SecInterval::CheckKind(ListExpr type, ListExpr& errorInfo) {
  return checkType(type);
}

size_t SecInterval::Sizeof() const {
  return 2 * sizeof(Instant) + 2 * sizeof(CcBool);
}

int SecInterval::Compare(const Attribute* attr) const {
   SecInterval* si = (SecInterval*) attr;
   if(!IsDefined()) {
     return si->IsDefined()? -1 : 0;
   }
   if(!si->IsDefined()){
     return 1;
  }
  return Interval<Instant>::CompareTo(*si);
}

ostream& SecInterval::Print(ostream &os) const {
  os << (lc?"[":"(");
  start.Print(os) << ", ";
  end.Print(os) << (rc?"]":")");
  return os;
}

bool SecInterval::Adjacent(const Attribute* attr) const {
  if(!IsDefined() || !attr->IsDefined())
    return false;
  return Interval<Instant>::Adjacent(*((Interval<Instant>*)attr));
}

SecInterval* SecInterval::Clone() const {
  SecInterval* i = new SecInterval(*this);
  return i;
}

size_t SecInterval::HashValue() const {
  return (this->start).HashValue() + (this->end).HashValue();
}

void SecInterval::CopyFrom(const Attribute *attr) {
  *this = *((SecInterval*)attr);
}

void SecInterval::WriteTo(char *dest) {
  strcpy(dest, ToString().c_str());
}

string SecInterval::ToString() {
  string result = "(" + start.ToString() + " " + end.ToString() + " "
      + (lc ? "TRUE" : "FALSE") + " " + (rc ? "TRUE" : "FALSE") + ")";
  return result;
}

ListExpr SecInterval::ToListExpr(const ListExpr typeInfo) const{
  if(IsDefined())
    return nl->FourElemList(start.ToListExpr(false), end.ToListExpr(false),
                            nl->BoolAtom(lc), nl->BoolAtom(rc));
  else
    return nl->SymbolAtom(Symbol::UNDEFINED());
}

SmiSize SecInterval::SizeOfChars() {
  string key = this->ToString();
  return (SmiSize)key.length();
}

bool SecInterval::ReadFrom(const ListExpr instance, const ListExpr typeInfo){
  if(listutils::isSymbolUndefined(instance)){
    SetDefined(false);
    return true;
  }
  if(!nl->HasLength(instance,4))
    return false;
  DateTime s(instanttype);
  DateTime e(instanttype);
  bool lc;
  bool rc;
  if (!s.ReadFrom(nl->First(instance), false)
   || !e.ReadFrom(nl->Second(instance), false))
    return false;
  ListExpr Lc = nl->Third(instance);
  if (nl->AtomType(Lc) != BoolType)
    return false;
  else
    lc = nl->BoolValue(Lc);
  ListExpr Rc = nl->Fourth(instance);
  if (nl->AtomType(Rc) != BoolType)
    return false;
  else
    rc = nl->BoolValue(Rc);
  return Set(s,e,lc,rc);
}

bool SecInterval::Set(const DateTime& s, const DateTime& e,
                      const bool lc, const bool rc) {
  if ((s < e) || ((s == e) && lc && rc)) {
    SetDefined(true);
    this->start = s;
    this->end = e;
    this->lc = lc;
    this->rc = rc;
    return true;
  }
  return false;
}

bool SecInterval::Set(const Interval<Instant>* iinst) {
  return Set(iinst->start, iinst->end, iinst->lc, iinst->rc);
}

bool SecInterval::SetStart(const DateTime& s, const bool lc) {
  if ((s < this->end) || ((s == this->end) && lc && this->rc)) {
    this->start = s;
    this->lc = lc;
    return true;
  }
  return false;
}

bool SecInterval::SetEnd(const DateTime& e, const bool rc) {
  if ((this->start < e) || ((this->start == e) && rc && this->lc)) {
    this->end = e;
    this->rc = rc;
    return true;
  }
  return false;
}

ListExpr SecInterval::Property(){
  return (nl->TwoElemList(
          nl->FiveElemList(
            nl->StringAtom("Signature"),
            nl->StringAtom("Example Type List"),
            nl->StringAtom("List Rep"),
            nl->StringAtom("Example List"),
            nl->StringAtom("Remarks")),
          nl->FiveElemList(
            nl->StringAtom("-> SecInterval"),
            nl->StringAtom(SecInterval::BasicType()),
            nl->StringAtom("(start end leftclosed rightclosed)"),
            nl->TextAtom
                ("2004-4-12-8:03:32.645 2011-07-01-08:55:22.000 TRUE FALSE"),
            nl->StringAtom("This type represents an interval"))));
}

const bool SecInterval::Contains(const Periods& per) const{
  if (!IsDefined() || !per.IsDefined())
    return false;
  int no = per.GetNoComponents();
  if (!no)
    return true;
  Interval<Instant> firstIv, lastIv;
  per.Get(0, firstIv);
  per.Get(no - 1, lastIv);
  return Interval<Instant>::Contains(firstIv)
      && Interval<Instant>::Contains(lastIv);
}

const bool SecInterval::Contains(const Interval<Instant>& si) const{
  if(!IsDefined())
     return false;
  return Interval<Instant>::Contains(si);
}
  
/*
5.47.2 Type mapping function for ~contains~

*/
ListExpr containsTM(ListExpr args) {
  if (nl->ListLength(args) != 2) {
    ErrorReporter::ReportError("Exactly two arguments required.");
    return nl->SymbolAtom(Symbol::TYPEERROR());
  }
  if (!SecInterval::checkType(nl->First(args))
   && !Periods::checkType(nl->First(args))) {
    ErrorReporter::ReportError
                   ("1st argument requires type interval or periods.");
    return nl->SymbolAtom(Symbol::TYPEERROR());
  }
  if (!SecInterval::checkType(nl->First(args))
   && !Periods::checkType(nl->First(args))) {
    ErrorReporter::ReportError
                   ("2nd argument requires type interval or periods.");
    return nl->SymbolAtom(Symbol::TYPEERROR());
  }
  return nl->SymbolAtom(CcBool::BasicType());
}

/*
5.47.3 Value mapping for operator ~contains~

*/
template<class C1, class C2>
int containsVM(Word* args, Word& result, int message, Word& local, Supplier s){
  C1* arg1 = static_cast<C1*>(args[0].addr);
  C2* arg2 = static_cast<C2*>(args[1].addr);
  result = qp->ResultStorage(s);
  CcBool* res = (CcBool*)result.addr;
  if(!arg1->IsDefined() || !arg2->IsDefined())
    res->SetDefined(false);
  else
    res->Set(true, arg1->Contains(*arg2));
  return 0;
}

/*
5.47.4 Value mapping array for operator ~contains~

*/
ValueMapping containsvm[] = {containsVM<SecInterval, SecInterval>,
                             containsVM<Periods, SecInterval>,
                             containsVM<SecInterval, Periods>,
                             containsVM<Periods, Periods>};

GenTC<SecInterval> interval;

/*
5.47.5 Specification for operator ~contains~

*/
const string ContainsSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\") "
  "( <text>periods x periods -> bool</text--->"
  "<text>_ contains _ </text--->"
  "<text>decides whether the first argument contains the second</text--->"
  "<text>p1 contains p2</text--->) )";

/*
5.47.6 Selection Function of operator ~contains~

*/
int containsSelect(ListExpr args) {
  if (SecInterval::checkType(nl->First(args))
   && SecInterval::checkType(nl->Second(args)))
    return 0;
  else if (Periods::checkType(nl->First(args))
        && SecInterval::checkType(nl->Second(args)))
    return 1;
  else if (SecInterval::checkType(nl->First(args))
        && Periods::checkType(nl->Second(args)))
    return 2;
  else if (Periods::checkType(nl->First(args))
        && Periods::checkType(nl->Second(args)))
    return 3;
  else
    return -1;
}

/*
5.47.7 Definition of operator ~contains~

*/
Operator temporalcontains("contains",
                          ContainsSpec,
                          4,
                          containsvm,
                          containsSelect,
                          containsTM);

/*
5.48 Operator ~swapcoord~

----
     mpoint -> mpoint
----

*/

/*
5.48.1 Type mapping function for ~swapcoord~

*/
ListExpr swapcoordTM(ListExpr args) {
  if (nl->ListLength(args) != 1) {
    return listutils::typeError("Exactly one argument required.");
  }
  if (!MPoint::checkType(nl->First(args))) {
    return listutils::typeError("Type mpoint required.");
  }
  return nl->SymbolAtom(MPoint::BasicType());
}

/*
5.48.2 Value mapping function for operator ~swapcoord~

*/
int swapcoordVM(Word* args, Word& result, int message, Word& local, Supplier s){
  MPoint* source = static_cast<MPoint*>(args[0].addr);
  result = qp->ResultStorage(s);
  MPoint* res = (MPoint*)result.addr;
  if (!source->IsDefined()) {
    res->SetDefined(false);
    return 0;
  }
  res->SetDefined(true);
  res->Clear();
  UPoint up(1);
  Point p0, p1;
  for (int i = 0; i < source->GetNoComponents(); i++) {
    source->Get(i, up);
    p0.Set(up.p0.GetY(), up.p0.GetX());
    p1.Set(up.p1.GetY(), up.p1.GetX());
    p0.SetDefined(true);
    p1.SetDefined(true);
    up.p0 = p0;
    up.p1 = p1;
    res->MergeAdd(up);
  }
  return 0;
}

/*
5.48.3 Specification for operator ~swapcoord~

*/
const string swapcoordSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\") "
  "( <text>mpoint -> mpoint</text--->"
  "<text>swapcoord( _ )</text--->"
  "<text>swaps the x and y coordinates of an mpoint</text--->"
  "<text>swapcoord(mpoint1)</text--->) )";

/*
5.48.4 Definition of operator ~swapcoord~

*/
Operator temporalswapcoord("swapcoord",
                           swapcoordSpec,
                           swapcoordVM,
                           Operator::SimpleSelect,
                           swapcoordTM);

/*
6 Creating the Algebra

*/

class TemporalUnitAlgebra : public Algebra
{
public:
  TemporalUnitAlgebra() : Algebra()
  {

    AddTypeConstructor(&interval);
    interval.AssociateKind("DATA");
    
    AddOperator( &temporalunitmakemvalue );
    AddOperator( &temporalunitthemvalue );
    AddOperator( &the_mvalue2 );
    AddOperator( &temporalunitqueryrect2d );
    AddOperator( &temporalunitpoint2d );
    AddOperator( &temporalunitisempty );
    AddOperator( &temporalunitdeftime );
    AddOperator( &temporalunitpresent );
    AddOperator( &temporalunitinitial );
    AddOperator( &temporalunitfinal );
    AddOperator( &temporalunitatinstant );
    AddOperator( &temporalunitatperiods );
    AddOperator( &temporalunitwhen );
    AddOperator( &temporalunitat );
    AddOperator( &temporalunitatmax );
    AddOperator( &temporalunitatmin );
    AddOperator( &temporalunitintersection );
    AddOperator( &temporalunitinside );
    AddOperator( &temporalunitpasses );
    AddOperator( &temporalunitget_duration );
    AddOperator( &temporalunittrajectory );
    AddOperator( &temporalunitdistance );
    AddOperator( &temporalunitabs );
    AddOperator( &temporalspeed );
    AddOperator( &temporalvelocity );
    AddOperator( &temporalderivable );
    AddOperator( &temporalderivative );
    AddOperator( &temporalunitnocomponents );
    AddOperator( &temporalunitnot );
    AddOperator( &temporalunitand );
    AddOperator( &temporalunitor );
    AddOperator( &temporalunitsometimes );
    AddOperator( &temporalunitnever );
    AddOperator( &temporalunitalways );
    AddOperator( &temporalunitisequal );
    AddOperator( &temporalunitisnotequal );
    AddOperator( &temporalunitsmaller );
    AddOperator( &temporalunitbigger );
    AddOperator( &temporalunitsmallereq );
    AddOperator( &temporalunitbiggereq );
    AddOperator( &temporalunitvalisequal );
    AddOperator( &temporalunitvalisnotequal );
    AddOperator( &temporalunitvalsmaller );
    AddOperator( &temporalunitvalbigger );
    AddOperator( &temporalunitvalsmallereq );
    AddOperator( &temporalunitvalbiggereq );
    AddOperator( &temporalunituint2ureal );
    AddOperator( &temporalunittheupoint );
    AddOperator( &temporalunittheivalue );
    AddOperator( &temporalunitlength );
    AddOperator( &temporalunitcanmeet);
    AddOperator( &atRectU);
    AddOperator(&temporalcontains);
    AddOperator(&temporalswapcoord);
  }
  ~TemporalUnitAlgebra() {};
};

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
  return (new TemporalUnitAlgebra());
}


