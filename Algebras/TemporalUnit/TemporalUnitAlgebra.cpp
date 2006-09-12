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

----
                            Signatur
  trajectory           upoint    -> line
  makemvalue           stream (tuple ([x1:t1,xi:uType,..,xn:tn]))  ->  mType
  size                 periods  -> real
  deftime      (*)     uT  -> periods
  atinstant    (*)     uT x instant  -> iT
  atperiods    (*)     uT x periods  -> (stream uT)
  Initial      (*)     uT -> iT
  final        (*)     uT  -> iT
  present      (*)     uT x instant  -> bool
                       uT x periods  -> bool
  point2d              periods  -> point
  queryrect2d          instant  -> rect
  speed                mpoint   -> mreal
                       upoint   -> ureal
  passes               upoint x point     -> bool
  at                   upoint x point     -> upoint
  circle               point x real x int -> region
  velocity             mpoint  -> mpoint
                       upoint  -> upoint
  derivable            mreal   -> mbool
                       ureal   -> ubool
  derivative           mreal   -> mreal
                       ureal   -> ureal

(*): These operators have been implemented for T in {bool, int, real, point}

----

*/

/*

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

So we are to implement:

----

State Operator/Signatures

OK   suse:  (stream X)            (map X Y)            --> (stream Y)
OK          (stream X)            (map X (stream Y))   --> (stream Y)

OK   suse2: (stream X) Y          (map X Y Z)          --> (stream Z)
OK          (stream X) Y          (map X Y stream(Z))  --> (stream Z)
OK          X          (stream Y) (map X y Z)          --> (stream Z)
OK          X          (stream Y) (map X y (stream Z)) --> (stream Z)
OK          (stream X) (stream Y) (map X Y Z)          --> (stream Z)
OK          (stream X) (stream Y) (map X Y (stream Z)) --> (stream Z)
            for X,Y,Z of kind DATA

OK   sfeed: T --> (stream T)                                   

OK   transformstream: (stream T) -> stream(tuple((element T)))
OK                    stream(tuple((id T))) -> (stream T)

OK   saggregate: (stream T) x (T x T --> T) x T  --> T

Test atmax: uT --> (stream uT)

Test atmin: uT --> (stream uT)

Test at:    ureal x real --> (stream ureal)

     distance:  T in {int, point}
Test           uT x uT -> ureal
Test           uT x  T -> ureal 
Test            T x uT -> ureal

pre  intersects: For T in {bool, int, string, real, point}:
                 uT x uT --> (stream ubool)

Test intersection:     uT x      uT --> (stream uT)
Test                   uT x       T --> (stream uT)
Test                    T x      uT --> (stream uT)
Pre                 ureal x   ureal --> (stream ureal)
Pre                upoint x uregion --> (stream upoint)

     udirection: upoint --> ureal

     no_components: uT --> uint

     area: uregion --> ureal

     and, or: ubool x ubool --> (stream ubool)
               bool x ubool --> (stream ubool)
              ubool x  bool --> (stream ubool)

     =, #: uT x uT --> (stream ubool)
  	    T x uT --> (stream ubool)
           uT x  T --> (stream ubool)

     initial, final: (stream uT) --> iT
     present: (stream uT) x instant --> bool
     present: (stream uT) x periods --> bool
     soemtimes: ubool --> bool
     never:     ubool --> bool
     always:    ubool --> bool



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

[TOC]

1 Overview

This file contains the implementation of the unit operators and
helping operators for indexing instant values in R-trees.

2 Defines, includes, and constants

*/


#include <cmath>
#include <stack>
#include <limits>

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
2.1 Definition of some constants

*/
const double MAXDOUBLE = numeric_limits<double>::max();
const double MINDOUBLE = numeric_limits<double>::min();


/*
3 Implementation of the unit class method operators

This section implements operators as member functions of the respective
unit class.

Definition of operators is similar to definition of type constructors. An
operator is defined by creating an instance of class ~Operator~. Again we
have to define some functions before we are able to create an ~Operator~
instance.

3.1 Operator ~speed~

*/
void MPoint::MSpeed( MReal& result ) const
{
  const UPoint *uPoint;

  UReal uReal;
  double x0, y0, x1, y1;
  Instant sup, inf;
  double t0, t1;
  double duration;

  result.Clear();

  if ( ! IsDefined() )
    result.SetDefined( false );

  else
    {
      result.StartBulkLoad();

      for( int i = 0; i < GetNoComponents(); i++ )
	{
	  Get( i, uPoint );
	  
	  x0 = uPoint->p0.GetX(); // initial pos
	  y0 = uPoint->p0.GetY();
	  
	  x1 = uPoint->p1.GetX(); // final pos
	  y1 = uPoint->p1.GetY();
	  
	  uReal.timeInterval = uPoint->timeInterval; // copy interval 
	  inf = uReal.timeInterval.start;            // for result 
	  sup = uReal.timeInterval.end;              // from argument
	  
	  t0 = inf.ToDouble(); // convert to milliseconds
	  t1 = sup.ToDouble();
	  
	  duration = (t1 - t0)/1000; // interval duration to seconds
	  
	  /*
	    The point unit can be represented as a function of
	    f(t) = (x0 + x1 * t, y0 + y1 * t).
	    The result of the derivation is the constant (x1,y1).
	    The speed is constant in each time interval.
	    Its value is represented by variable c. The variables a and b  
	    are set to zero.
	    
	  */
	  uReal.a = 0;
	  uReal.b = 0;
	  uReal.c = sqrt(pow( (x1-x0), 2 ) + pow( (y1- y0), 2 ))/duration;
	  uReal.r = false;
	  
	  result.Add( uReal ); // append ureal to mreal
	}
      result.EndBulkLoad( false );
    }
}
void UPoint::USpeed( UReal& result ) const
{

  double x0, y0, x1, y1;
  Instant sup, inf;
  double t0, t1;
  double duration;

  if ( ! IsDefined() )
    result.SetDefined( false );
  else
    {
      
      x0 = p0.GetX();
      y0 = p0.GetY();
      
      x1 = p1.GetX();
      y1 = p1.GetY();
      
      result.timeInterval = timeInterval;
      
      if (result.IsDefined() )
	{
	  inf = result.timeInterval.start;
	  sup = result.timeInterval.end;
	  
	  t0 = inf.ToDouble();
	  t1 = sup.ToDouble();
	  
	  duration = (t1 - t0)/1000;   // value in seconds
	  
	  result.a = 0;                // speed is constant in the interval
	  result.b = 0;
	  result.c = sqrt(pow( (x1-x0), 2 ) + pow( (y1- y0), 2 ))/duration;
	  result.r = false;
	}
    }
}

/*
3.2 Operator ~Velocity~

*/
void MPoint::MVelocity( MPoint& result ) const
{
  const UPoint *uPoint;

  double x0, y0, x1, y1;
  Instant sup, inf;
  double t0, t1;
  double duration;

  result.Clear();

  if ( ! IsDefined() )
    result.SetDefined( false );
  else
    {
      result.StartBulkLoad();

      for( int i = 0; i < GetNoComponents(); i++ )
	{
	  Get( i, uPoint );
	  
	  x0 = uPoint->p0.GetX(); // initial coordinates
	  y0 = uPoint->p0.GetY();
	  
	  x1 = uPoint->p1.GetX(); // final coordinates
	  y1 = uPoint->p1.GetY();
	  
	  inf = uPoint->timeInterval.start;
	  sup = uPoint->timeInterval.end;
	  
	  t0 = inf.ToDouble();
	  t1 = sup.ToDouble();
	  
	  duration = (t1 - t0)/1000;    // value in second
	  
	  //  create an interval:
	  Interval<Instant> iv(uPoint->timeInterval.start,
			       uPoint->timeInterval.end,
			       uPoint->timeInterval.lc,
			       uPoint->timeInterval.rc);
	  
	  /*
	    Definition of a new point unit p. The velocity is constant
	    at all times of the interval of the unit. This is exactly
	    the same as within operator ~speed~. The result is a vector
	    and can be represented as a upoint.
	    
	  */
	  
	  UPoint p(iv,(x1-x0)/duration,0,(y1- y0)/duration,0);
	  
	  result.Add( p );
	}
      result.EndBulkLoad( false );
    }
}

void UPoint::UVelocity( UPoint& result ) const
{

  double x0, y0, x1, y1;
  Instant sup, inf;
  double t0, t1;
  double duration;

  if ( ! IsDefined() )
    result.SetDefined( false );
  else
    {
      x0 = p0.GetX();
      y0 = p0.GetY();
  
      x1 = p1.GetX();
      y1 = p1.GetY();
  
      result.timeInterval = timeInterval;
      
      if (result.IsDefined() )
	{
	  inf = result.timeInterval.start;
	  sup = result.timeInterval.end;
	  
	  t0 = inf.ToDouble();
	  t1 = sup.ToDouble();

	  duration = (t1 - t0)/1000;   // value in second

	  Interval<Instant> iv(result.timeInterval.start,
			       result.timeInterval.end,
			       result.timeInterval.lc,
			       result.timeInterval.rc);

	  UPoint p(iv,(x1-x0)/duration,0,(y1-y0)/duration,0);
	  
	  result.CopyFrom( &p );
	  
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

  line.Clear();

  if ( ! IsDefined() )
    line.SetDefined( false );
  else 
    {
      
      line.StartBulkLoad();
      
      HalfSegment hs;
      
      if( !AlmostEqual( p0, p1 ) )
	{
	  
	  hs.Set( true, p0, p1 );

	  line += hs;
	  hs.SetLeftDomPoint( !hs.IsLeftDomPoint() );
	  line += hs;
	}
      
      line.EndBulkLoad();
    }
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
  result = qp->ResultStorage( s );
  MPoint* input = (MPoint*)result.addr;
  
  if ( input->IsDefined() )
    // call member function:
    ((MPoint*)args[0].addr)->MSpeed( *((MReal*)result.addr) ); 
  else
    ((MPoint*)args[0].addr)->SetDefined(false);

  return 0;
}

int UnitPointSpeed(Word* args,Word& result,int message,Word& local,Supplier s)
{
  result = qp->ResultStorage( s );
  UPoint* input = (UPoint*)result.addr;
  
  if ( input->IsDefined() )
    // call member function:
    ((UPoint*)args[0].addr)->USpeed(  *((UReal*)result.addr) );
  else
     ((UPoint*)args[0].addr)->SetDefined(false);
  return 0;
}

/* 
5.1.3 Specification for operator ~speed~

*/

const string
TemporalSpecSpeed  =
"( ( \"Algebra\" \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>TemporalUnitAlgebra</text--->"
"<text>mpoint -> mreal\n"
"upoint -> ureal</text--->"
"<text>speed( _ )</text--->"
"<text>return the speed of a temporal spatial object.</text--->"
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
"( ( \"Algebra\" \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>TemporalUnitAlgebra</text--->"
"<text>(instant) -> rect</text--->"
"<text>queryrect2d( _ )</text--->"
"<text>return the rect of an instant object for a time interval.</text--->"
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
	  
	  Interval<Instant> timeInterval(intv1->start,intv2->end,
					 intv1->lc,intv2->rc);
	  
	  sup = timeInterval.end;
	  inf = timeInterval.start;
	  Y = sup.ToDouble(); // Derives the maximum of all intervals.
	  X = inf.ToDouble(); // Derives the minimum of all intervals.
	}
      ((Point*)result.addr)->Set(X,Y ); // Returns the calculated point.
    }
  
  return 0;
}


/*
5.3.3 Specification for operator ~point2d~

*/
const string
TemporalSpecPoint2d  =
"( ( \"Algebra\" \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>TemporalUnitAlgebra</text--->"
"<text>(periods) -> point</text--->"
"<text>point2d( _ )</text--->"
"<text>return the point of a given interval.</text--->"
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
  double res, duration, intervalue_sup, intervalue_inf;
  Instant sup, inf;

  result = qp->ResultStorage( s );
  Periods* range = (Periods*)args[0].addr;

  if ( !range->IsDefined() )
    ((CcReal*)result.addr)->SetDefined( false );
  else 
    {
      if( !range->IsEmpty()  )
	{
	  const Interval<Instant> *intv1, *intv2;
	  duration = 0;
      
	  for( int i = 0; i < range->GetNoComponents(); i++ )
	    {
	      range->Get( i, intv1 );
	      range->Get( i, intv2 );
	      
	      Interval<Instant> timeInterval(intv1->start,intv2->end,
					     intv1->lc, intv2->rc);
	  
	      sup = timeInterval.end;
	      inf = timeInterval.start;
	      
	      intervalue_sup = sup.ToDouble();
	      intervalue_inf = inf.ToDouble();
	      // summarize all time intervals in seconds:
	      duration += (intervalue_sup - intervalue_inf)/1000;
	    }
	  res = duration;
	}
      ((CcReal*)result.addr)->Set(true, res);  // return the resulT
    }
  return 0;
}

/*
5.4.3 Specification for operator ~size~

*/
const string
TemporalSpecSize  =
"( ( \"Algebra\" \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>TemporalUnitAlgebra</text--->"
"<text>(periods) -> real</text--->"
"<text>size( _ )</text--->"
"<text>return the duration of a moving object.</text--->"
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
	       || inputtype == "upoint" ),
	      "Attribute type is not of type ubool, uint, ureal or upoint.");
  
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
	  unit = (Unit*) currentAttr;
	  m->Add( *unit );
	  currentTuple->DeleteIfAllowed();
	}
      qp->Request(args[0].addr, currentTupleWord);
    }
  m->EndBulkLoad( false );
  
  qp->Close(args[0].addr);

  return 0;

}

/*
5.5.3 Specification for operator ~makemvalue~

*/
const string
TemporalSpecMakemvalue  =
"( ( \"Algebra\" \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>TemporalUnitAlgebra</text--->"
"<text>Fot T in {bool, int, real, point}:"
"((stream (tuple ((x1 t1)...(xn tn)))"
" (uT)))-> mT</text--->"
"<text>makemvalue[ _ ]</text--->"
"<text>Create a moving object from a tuple stream containing "
"units.</text--->"
"<text>makemvalue[ u1 ]</text---> ) )";

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


  return -1; // This point should never be reached
}

ValueMapping temporalmakemvaluemap[] = { MappingMakemvalue<MBool, UBool>,
                                         MappingMakemvalue<MBool, UBool>,
                                         MappingMakemvalue<MReal, UReal>,
                                         MappingMakemvalue<MPoint, UPoint> };

/*
5.5.5  Definition of operator ~makemvalue~

*/
Operator temporalunitmakemvalue( "makemvalue",
                        TemporalSpecMakemvalue,
                        4,
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

  Line *line = ((Line*)result.addr);
  UPoint *upoint = ((UPoint*)args[0].addr);

  if ( !upoint->IsDefined() )
    line->SetDefined( false );
  else 
    upoint->UTrajectory( *line );   // call memberfunction
  
  return 0;
}

/*
5.6.3 Specification for operator ~trajectory~

*/
const string
TemporalSpecTrajectory  =
"( ( \"Algebra\" \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>TemporalUnitAlgebra</text--->"
"<text>upoint -> line</text--->"
"<text>trajectory( _ )</text--->"
"<text>get the trajectory of the corresponding"
"unit point object.</text--->"
"<text>trajectory( up1 )</text---> ) )";

/* 
5.6.4 Selection Function of operator ~trajectory~

NOt necessary.

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
"( ( \"Algebra\" \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>TemporalUnitAlgebra</text--->"
"<text>uT -> periods \n"
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
      pResult->SetDefined( true );
      posUnit->TemporalFunction( *t, pResult->value );
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
"( ( \"Algebra\" \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>TemporalUnitAlgebra</text--->"
"<text>(uT instant) -> iT\n"
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
      ErrorReporter::ReportError("Operator atperiods expect a first argument "
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
  Word uWord;     // the address of the unit point/int/real/string value
  Word pWord;    //  the adress of the period value
  int  j;       //   save the number of the interval
};

/*
Variant 1: first argument is a scalar value

*/

template <class Mapping>
int MappingUnitAtPeriods( Word* args, Word& result, int message,
                          Word& local, Supplier s )
{
  AtPeriodsLocalInfo *localinfo;
  const Interval<Instant> *interval;

  const Mapping* unit;
  Mapping r;
  Periods* periods;


  switch( message )
  {
  case OPEN:

    localinfo = new AtPeriodsLocalInfo;
    qp->Request(args[0].addr, localinfo->uWord);
    qp->Request(args[1].addr, localinfo->pWord);
    localinfo->j = 0;
    local = SetWord(localinfo);
    return 0;

  case REQUEST:
    
    if( local.addr == 0 )
      return CANCEL;
    localinfo = (AtPeriodsLocalInfo *)local.addr;
    unit = (Mapping*)localinfo->uWord.addr;
    periods = (Periods*)localinfo->pWord.addr;
    
    if( localinfo->j == periods->GetNoComponents() )
      return CANCEL;
    periods->Get( localinfo->j, interval );
    
    if( interval->Before( unit->timeInterval ) )
      {
	while (1)
	  {
	    if( ++localinfo->j == periods->GetNoComponents() )
	      break;
	    periods->Get(localinfo->j, interval);
	    if (!( interval->Before( unit->timeInterval )))
	      break;
	  }
      }

    if( localinfo->j >= periods->GetNoComponents() ) {
      result.addr = 0;
      return CANCEL;
    }
    
    if( unit->timeInterval.Before( *interval ) )
      {
	result.addr = 0;
	return CANCEL;
      }
    else
      {
	unit->AtInterval( *interval, r );
	Mapping* aux = new Mapping( r );
	result = SetWord( aux );
	localinfo->j++;
	return YIELD;
      }
    
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


template <class Mapping>
int MappingUnitStreamAtPeriods( Word* args, Word& result, int message,
                                Word& local, Supplier s )
{
  AtPeriodsLocalInfoUS *localinfo;
  Mapping *unit, *aux;
  Mapping resultUnit;
  Periods *periods;
  const Interval<Instant> *interval;

  switch( message )
  { 
  case OPEN:
    localinfo = new AtPeriodsLocalInfoUS;
    qp->Request(args[1].addr, localinfo->pWord);   // get address of periods
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
    unit = (Mapping *) localinfo->uWord.addr;
    if ( localinfo->pWord.addr == 0 )
      { result.addr = 0; return CANCEL; }
    periods = (Periods *) localinfo->pWord.addr;
    
    // search for a pair of overlapping unit/interval:
    while (1)
      {
	if ( localinfo->j == periods->GetNoComponents() ) // redo first interval
	  { localinfo->j = 0;
	    unit->DeleteIfAllowed();                // delete original unit?
	    qp->Request(args[0].addr, localinfo->uWord);  // get new unit
	    if( qp->Received( args[0].addr ) )
	      unit = (Mapping *) localinfo->uWord.addr;
	    else 
	      { result.addr = 0; return CANCEL; }   // end of unit stream
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
    aux = new Mapping( resultUnit );
    result = SetWord( aux );
    localinfo->j++;                           // increase interval counter
    return YIELD;
    
  case CLOSE:
    if ( local.addr != 0 )
      {
	localinfo = (AtPeriodsLocalInfoUS *) local.addr;
	if ( localinfo->uWord.addr != 0 )
	  {
	    unit = (Mapping *) localinfo->uWord.addr;
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
"( ( \"Algebra\" \"Signature\" \"Signature (1)\" \"Signature (2)\" \"Syntax\""
" \"Meaning\" \"Example (1)\" \"Example (2)\") "
"( <text>TemporalUnitAlgebra</text--->"
"<text>For T in {int, bool, real, string, point, region}:</text--->"
"<text>(uT periods) -> stream uT\n</text--->"
"<text>((stream uT) periods) -> stream uT</text--->"
"<text>_ atperiods _ </text--->"
"<text>restrict the movement to the given"
" periods.</text--->"
"<text>upoint1 atperiods thehour(2003,11,11,8)</text--->"
"<text>sfeed(upoint1) atperiods thehour(2003,11,11,8)</text--->) )";

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
    res->SetDefined( true );
    unit->TemporalFunction( unit->timeInterval.start, res->value );
    res->instant.CopyFrom( &unit->timeInterval.start );
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
    res->SetDefined( true );
    unit->TemporalFunction( unit->timeInterval.end, res->value );
    res->instant.CopyFrom( &unit->timeInterval.end );
   }
  return 0;
}

/*
5.10.3 Specification for operator ~initial~ and ~final~

*/
const string
TemporalSpecInitial  =
"( ( \"Algebra\" \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>TemporalUnitAlgebra</text--->"
"<text>uT -> iT\n"
"(T in {bool, int, real, string, point, region})</text--->"
"<text>initial( _ )</text--->"
"<text>From a unit type, get the intime value corresponding"
" to the initial instant.</text--->"
"<text>initial( upoint1 )</text---> ) )";

const string
TemporalSpecFinal  =
"( ( \"Algebra\" \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>TemporalUnitAlgebra</text--->"
"<text>uT -> iT\n"
"(T in {bool, int, real, string, point, region})</text--->"
"<text>final( _ )</text--->"
"<text>get the intime value corresponding"
" to the final instant.</text--->"
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

  Periods defTime( 0 );

  periods->Clear();
  periods->StartBulkLoad();
  periods->Add( m->timeInterval );
  periods->EndBulkLoad( false );
  periods->Merge( defTime );

  if( periods->IsEmpty() )
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
"( ( \"Algebra\" \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>TemporalUnitAlgebra</text--->"
"<text>({m|u}T instant) -> bool\n"
"({m|u}T periods) -> bool\n"
"(T in {bool, int, real, string, point, region)</text--->"
"<text>_ present _ </text--->"
"<text>whether the moving/unit object is present at the"
" given instant or period.</text--->"
"<text>mpoint1 present instant1</text---> ) )";

/* 
5.11.4 Selection Function of operator ~present~

*/

int
UnitInstantPeriodsSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args ),
           arg2 = nl->Second( args );


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
"( ( \"Algebra\" \"Signature\" \" \" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>TemporalUnitAlgebra</text--->"
"<text>(uT T) -> bool\n</text--->"
"<text>for T in {bool, int, real*, string, point, region*}\n"
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

  //  qp->Request(args[0].addr, a0);
  //  qp->Request(args[1].addr, a1);
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
  

  switch (message)
    {
    case OPEN :
      
      cout << "\nMappingUnitAt_r: OPEN" << endl;
      localinfo = new MappingUnitAt_rLocalInfo;
      localinfo->finished = true;
      localinfo->NoOfResults = 0;
      cout << "  1" << endl;

      qp->Request(args[0].addr, a0);
      uinput = (UReal*)(a0.addr);
      cout << "  1.1" << endl;

      qp->Request(args[1].addr, a1);
      value = (CcReal*)(a1.addr);
      cout << "  1.2" << endl;

      cout << "  2" << endl;

      cout << "  2.1: " << uinput->IsDefined() << endl;
      cout << "  2.2: " << value->IsDefined() << endl;

      if ( !uinput->IsDefined() ||
	   !value->IsDefined() )
	{ // some input is undefined -> return empty stream
	  cout << "  3: Some input is undefined. No result." << endl;
	  localinfo->NoOfResults = 0;
	  localinfo->finished = true;
	  local = SetWord(localinfo);
	  cout << "\nMappingUnitAt_r: finished OPEN (1)" << endl;
	  return 0;
	}
      cout << "  4" << endl;

      y = value->GetRealval();
      a = uinput->a;
      b = uinput->b;
      c = uinput->c;
      r = uinput->r;
      deftime = uinput->timeInterval;

      cout << "    The UReal is" << " a= " << a << " b= " 
	   << b << " c= " << c << " r= " << r << endl;
      cout << "    The Real is y=" << y << endl;
      cout << "  5" << endl;
	    
      if ( (a == 0) && (b == 0) )
	{ // constant function. Possibly return input unit
	  cout << "  6: 1st arg is a constant value" << endl;
	  if (c != y)
	    { // There will be no result, just an empty stream
	      cout << "  7" << endl;
	      localinfo->NoOfResults = 0;
	      localinfo->finished = true;
	    }
	  else
		{ // Return the complete unit
		  cout << "  8: Found constant solution" << endl;
		  cout << "    T1=" << c << endl;
		  cout << "    Tstart=" << deftime.start.ToDouble() << endl;
		  cout << "    Tend  =" << deftime.end.ToDouble() << endl;
		  (UReal*)(localinfo->runits[localinfo->NoOfResults].addr)
		    = uinput->Copy();
		  localinfo->NoOfResults++;
		  localinfo->finished = false;
		  cout << "  9" << endl;
		}
	  cout << "  10" << endl;
	  local = SetWord(localinfo);
	  cout << "\nMappingUnitAt_r: finished OPEN (2)" << endl;
	  return 0;
	}
      if ( (a == 0) && (b != 0) )
	{ // linear function. Possibly return input unit restricted 
	  // to single value
	  cout << "  11: 1st arg is a linear function" << endl;
	  double T1 = (y - c)/b;
	  cout << "    T1=" << T1 << endl;	  
	  cout << "    Tstart=" << deftime.start.ToDouble() << endl;
	  cout << "    Tend  =" << deftime.end.ToDouble() << endl;	  
	  t1.ReadFrom( T1 + deftime.start.ToDouble() );
	  if (deftime.Contains(t1))
	    { // value is contained by deftime
	      cout << "  12: Found valid linear solution." << endl;
	      (UReal*)(localinfo->runits[localinfo->NoOfResults].addr) = 
		uinput->Copy();
	      ((UReal*)(localinfo
			->runits[localinfo->NoOfResults].addr))
		->timeInterval = Interval<Instant>(t1, t1, true, true);
	      localinfo->NoOfResults++;
	      localinfo->finished = false;		  
	      cout << "  13" << endl;
	    }
	  else
	    { // value is not contained by deftime -> no result
	      cout << "  14: Found invalid linear solution." << endl;
	      localinfo->NoOfResults = 0;
	      localinfo->finished = true;
	      cout << "  15" << endl;
	    }
	  cout << "  16" << endl;
	  local = SetWord(localinfo);
	  cout << "\nMappingUnitAt_r: finished OPEN (3)" << endl;
	  return 0;
	}
      
      cout << "  17" << endl;
      radicand = (b*b + 4*a*(y-c));
      if ( (a != 0) && (radicand >= 0) )
	{ // quadratic function. There are possibly two result units
	  // calculate the possible t-values t1, t2
	  
/*
The solution to the equation $at^2 + bt + c = y$ is 
\[t_{1,2} = \frac{-b \pm \sqrt{b^2-4a(c-y)}}{2a},\] for $b^2-4a(c-y) = b^2+4a(y-c) \geq 0$.


*/
	  cout << "  18: 1st arg is a quadratic function" << endl;
	  double T1 = (-b + sqrt(radicand)) / (2*a);
	  double T2 = (-b - sqrt(radicand)) / (2*a);
	  cout << "    T1=" << T1 << endl;
	  cout << "    T2=" << T2 << endl;
	  cout << "    Tstart=" << deftime.start.ToDouble() << endl;
	  cout << "    Tend  =" << deftime.end.ToDouble() << endl;	  
	  t1.ReadFrom( T1 + deftime.start.ToDouble() );
	  t2.ReadFrom( T2 + deftime.start.ToDouble() );
	  
	  // check, whether t1 contained by deftime
	  if (deftime.Contains( t1 ))
	    {
	      cout << "  19: Found first quadratic solution" << endl;
	      rdeftime.start = t1;
	      rdeftime.end = t1;
	      localinfo->runits[localinfo->NoOfResults].addr = 
		new UReal( rdeftime,a,b,c,r );
	      localinfo->NoOfResults++;
	      localinfo->finished = false;
	      cout << "  20" << endl;
	    }
	  // check, whether t2 contained by deftime
	  if ( !(t1 == t2) && (deftime.Contains( t2 )) )
	    {
	      cout << "  21: Found second quadratic solution" << endl;
	      rdeftime.start = t2;
	      rdeftime.end = t2;
	      localinfo->runits[localinfo->NoOfResults].addr = 
		new UReal( rdeftime,a,b,c,r );
	      localinfo->NoOfResults++;
	      localinfo->finished = false;
	      cout << "  22" << endl;
	    }
	}
      else // negative discreminant -> there is no real solution 
	   //                          and no result unit
	{
	  cout << "  23: No real-valued solution" << endl;
	  localinfo->NoOfResults = 0;
	  localinfo->finished = true;
	  cout << "  24" << endl;
	}
      cout << "  25" << endl;
      local = SetWord(localinfo);
      cout << "\nMappingUnitAt_r: finished OPEN (4)" << endl;
      return 0;
      
    case REQUEST :
      
      cout << "\nMappingUnitAt_r: REQUEST" << endl;
      if (local.addr == 0)
	{
	  cout << "\nMappingUnitAt_r: finished REQUEST CANCEL (1)" << endl;
	  return CANCEL;
	}
      localinfo = (MappingUnitAt_rLocalInfo*) local.addr;
      cout << "\n   localinfo: finished=" << localinfo->finished 
	   << " NoOfResults==" << localinfo->NoOfResults << endl;

      if (localinfo->finished)
	{
	  cout << "\nMappingUnitAt_r: finished REQUEST CANCEL (2)" << endl;
	  return CANCEL;
	}
      if ( localinfo->NoOfResults <= 0 )
	{ localinfo->finished = true;
	  cout << "\nMappingUnitAt_r: finished REQUEST CANCEL (3)" << endl;
	  return CANCEL;
	}
      localinfo->NoOfResults--;
      result = SetWord( ((UReal*)(localinfo
				  ->runits[localinfo->NoOfResults].addr))
			->Clone() );
      ((UReal*)(localinfo->runits[localinfo->NoOfResults].addr))
	->DeleteIfAllowed();
      cout << "\nMappingUnitAt_r: finished REQUEST YIELD" << endl;
      return YIELD;
      
    case CLOSE :

      cout << "\nMappingUnitAt_r: CLOSE" << endl;
      if (local.addr != 0)
	{
	  localinfo = (MappingUnitAt_rLocalInfo*) local.addr;
	  for (;localinfo->NoOfResults>0;localinfo->NoOfResults--)
	    ((UReal*)(localinfo->runits[localinfo->NoOfResults].addr))
	      ->DeleteIfAllowed();
	  delete localinfo;
	}
      cout << "\nMappingUnitAt_r: finished CLOSE" << endl;
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
"( ( \"Algebra\" \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>TemporalUnitAlgebra</text--->"
"<text>For T in {bool, int, string, point, region*}:\n"
"(uT    T   ) -> uT\n"
"(ureal real) -> (stream ureal)\n"
"(*): Not yet implemented</text--->"
"<text> _ at _ </text--->"
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

  res->Clear();

  if (!p->IsDefined() || !r->IsDefined() || !narg->IsDefined() )
    {
      res->SetDefined( false );
    }
  else
    {
      x = p->GetX();
      y = p->GetY();
      
      n = narg->GetIntval();
      radius = r->GetRealval();
      
      res->StartBulkLoad();
      
      // Definition of an empty region:
      Region rg(0);
      HalfSegment hs;
      
      
      if (( p->IsDefined())&&(n>3)&&(n<100)&&(radius >0.0))
	{
	  
	  //  Determination of a n polygon.
	  //  Division of 360 degree in n parts with the help of
	  //  a standardised circle and the circumference. U = 2 * PI
	  
	  for( int i = 0; i < n; i++ )
	    {
	      angle = i * 2 * PI/n;
	      valueX = x + radius * cos(angle);
	      valueY = y + radius * sin(angle);
	      
	      //  The first point of the segment of a region.
	      //  The x-value can be defined with the cosine and
	      //  the y-value with the sine.
	      
	      Point edge1(true, valueX ,valueY);
	      
	      hs.attr.faceno = 0;
	      hs.attr.cycleno = 0;
	      hs.attr.edgeno = partnerno;
	      hs.attr.partnerno = partnerno++;
	      
	      if ((i+1) >= n)
		angle = 0 * 2 * PI/n;
	      else
		angle = (i+1) * 2 * PI/n;
	      
	      valueX = x + radius * cos(angle);
	      valueY = y + radius * sin(angle);
	      // The second point of the segment of a region.
	      Point edge2(true, valueX ,valueY);
	      
	      // Definition of the halfsegments.
	      hs.Set(true, edge1, edge2);
	      *res += hs;
	      hs.SetLeftDomPoint( !hs.IsLeftDomPoint() );
	      *res += hs;	  
	    }      
	}
      res->EndBulkLoad(true);
      res->SetPartnerNo();
      res->ComputeRegion();
    }
  return 0;
}

/*
5.14.3 Specification for operator ~circle~

*/
const string
TemporalSpecCircle =
"( ( \"Algebra\" \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>TemporalUnitAlgebra</text--->"
"<text>(point real int) -> region</text--->"
"<text>circle ( _ ) </text--->"
"<text>defines a circle with a given radius"
" and n calculated points.</text--->"
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
"( ( \"Algebra\" \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>TemporalUnitAlgebra</text--->"
"<text>int x int -> point</text--->"
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

----  mpoint  ->  mpoint

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
  result = qp->ResultStorage( s );
  MPoint* input = (MPoint*)args[0].addr;
  
  if ( !input->IsDefined() )
    ((MPoint*)args[0].addr)->SetDefined( false );
  else 
    ((MPoint*)args[0].addr)->MVelocity(  *((MPoint*)result.addr) );

  return 0;
}

int UnitPointVelocity(Word* args, Word& result, int message,
                      Word& local, Supplier s)
{
  result = qp->ResultStorage( s );
  UPoint* input = (UPoint*)args[0].addr;

  if ( !input->IsDefined() )
    ((UPoint*)args[0].addr)->SetDefined( false );
  else
    ((UPoint*)args[0].addr)->UVelocity(  *((UPoint*)result.addr) );

  return 0;
}

/*
5.16.3 Specification for operator ~velocity~

*/
const string
TemporalSpecVelocity=
"( ( \"Algebra\" \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>TemporalUnitAlgebra</text--->"
"<text>mpoint -> mpoint\n"
"upoint -> upoint</text--->"
"<text>velocity ( _ ) </text--->"
"<text>describes the vector of the speed"
" to the given temporal spatial object.</text--->"
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
"( ( \"Algebra\" \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>TemporalUnitAlgebra</text--->"
"<text>mreal -> mbool\n"
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
	  
	  // FALSE means in this case that a real unit describes a quadratic
	  // polynomial. A derivation is possible. 
	  // The polynom looks like at^2 + bt + c.
	  // The derivative of this polynom is 2at + b.
	  if (Unit->r == false)
	    {
	      uReal.timeInterval = Unit->timeInterval;
	      uReal.a = 0;
	      uReal.b = 2 * Unit->a;
	      uReal.c = Unit->b;
	  uReal.r = Unit->r;
	    }
	  else
	    {
	      // derivation of the real unit is not possible
	      uReal.SetDefined(false);
	    }
	  
	  res->Add( uReal );
	}
      res->EndBulkLoad( false );
    }
  else // value is undefined
    res->SetDefined( false );
  return 0;
}

int UnitPointDerivative( Word* args, Word& result, int message,
                         Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  UReal* Unit = (UReal*)args[0].addr;
  UReal* res = ((UReal*)result.addr);


  if (Unit->IsDefined())
    {
      if (Unit->r == false)
	{
	  res->timeInterval = Unit->timeInterval;
	  res->a = 0;
	  res->b = 2 * Unit->a;
	  res->c = Unit->b;
	  res->r = Unit->r;
	}
      else
	{
	  res->SetDefined(false);
	}
      
    }
  else // Unit is undefines
    res->SetDefined(false);
  return 0;
}

/*
5.18.3 Specification for operator ~derivative~

*/
const string
TemporalSpecDerivative=
"( ( \"Algebra\" \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>TemporalUnitAlgebra</text--->"
"<text>{m|u}real -> {m|u}real</text--->"
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
      qp->Request(args[0].addr, argValue);
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
"( ( \"Algebra\" \"Signature\" \" \" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>TemporalUnitAlgebra</text--->"
"<text>For T in kind DATA </text--->"
"<text>T -> (stream T)</text--->"
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

  cout << "TypeMapSuse2: 7" << endl;

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
      
      // cout << "Suse_SN received OPEN" << endl;
      sli = new SuseLocalInfo;
      sli->Xfinished = true;
      qp->Open(instream.addr);
      sli->Xfinished = false;
      local = SetWord(sli);
      // cout << "Suse_SN finished OPEN" << endl;
      return 0;
      
    case REQUEST :
      
      // For each REQUEST, we get one value from the stream,
      // pass it to the parameter function and evalute the latter.
      // The result is simply passed on.

      // cout << "Suse_SN received REQUEST" << endl;
      if( local.addr == 0 )
	{
	  // cout << "Suse_SN finished REQUEST: CANCEL (1)" << endl;  
	  return CANCEL;
	}
      sli = (SuseLocalInfo*)local.addr;
      
      if (sli->Xfinished)
	{
	  // cout << "Suse_SN finished REQUEST: CANCEL (2)" << endl;  
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
	  //cout << "        result.addr    =" << result.addr << endl;
	  argValue.addr = 0;
	  // cout << "Suse_SN finished REQUEST: YIELD" << endl;
	  return YIELD;
	}
      else // (input stream consumed completely)
	{
	  qp->Close(instream.addr);
	  sli->Xfinished = true;
	  result.addr = 0;
	  // cout << "Suse_SN finished REQUEST: CANCEL (3)" << endl;  
	  return CANCEL;
	}
      
    case CLOSE :
      
      // cout << "Suse_SN received CLOSE" << endl;
      if( local.addr != 0 )
	{
	  sli = (SuseLocalInfo*)local.addr;
	  if ( !sli->Xfinished )
	    qp->Close( instream.addr );
	  delete sli;
	}
      // cout << "Suse_SN finished CLOSE" << endl;
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
    
    // cout << "\nSuse_SS: Received REQUEST";
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
	    // cout << "     result.addr=" << result.addr << endl;
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
  Word              argConfDescriptor;  
  ArgVectorPointer  funargs;

  switch (message)
    {
    case OPEN :
      
      // cout << "\nSuse_SNN received OPEN" << endl;
      sli = new SuseLocalInfo ;
      sli->Xfinished = true;
      sli->X.addr = 0;
      sli->Y.addr = 0;  
      sli->fun = SetWord(args[2].addr); 
      // get argument configuration info
      qp->Request(args[3].addr, argConfDescriptor);      
      sli->argConfDescriptor = ((CcInt*)argConfDescriptor.addr)->GetIntval();
      if(sli->argConfDescriptor & 4)
	{ 
	  delete( sli );
	  local.addr = 0;
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
      qp->Request(sli->Y.addr, sli->Y);   // save value of constant argument

      local = SetWord(sli);
      // cout << "Suse_SNN finished OPEN" << endl;
      return 0;
      
    case REQUEST :
      
      // For each REQUEST, we get one value from the stream,
      // pass it (and the remaining constant argument) to the parameter 
      // function and evalute the latter. The result is simply passed on.
      // sli->X is the stream, sli->Y the constant argument.

      //cout << "Suse_SNN received REQUEST" << endl;

      // 1. get local data object
      if (local.addr == 0)
	{
	  result.addr = 0;
	  // cout << "Suse_SNN finished REQUEST: CLOSE (1)" << endl;
	  return CANCEL;
	}
      sli = (SuseLocalInfo*) local.addr;
      if (sli->Xfinished)
	{ // stream already exhausted earlier
	  result.addr = 0;
	  // cout << "Suse_SNN finished REQUEST: CLOSE (2)" << endl;
	  return CANCEL;
	}
      
      // 2. request value from outer stream
      qp->Request( sli->X.addr, xval );
      if(!qp->Received( sli->X.addr ))
	{ // stream exhausted now
	  qp->Close( sli->X.addr );
	  sli->Xfinished = true;
	  // cout << "Suse_SNN finished REQUEST: CLOSE (3)" << endl;
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
      //cout << "     result.addr=" << result.addr << endl;
      ((Attribute*) (xval.addr))->DeleteIfAllowed(); 
      //cout << "Suse_SNN finished REQUEST: YIELD" << endl;
      return YIELD;

    case CLOSE :
      
      //cout << "Suse_SNN received CLOSE" << endl;
      if( local.addr != 0 )
	{
	  sli = (SuseLocalInfo*)local.addr;
	  if (!sli->Xfinished)
	    qp->Close( sli->X.addr ); // close input
	  delete sli;
	}
      //cout << "Suse_SNN finished CLOSE" << endl;
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
  Word              argConfDescriptor;  
  ArgVectorPointer  funargs;

  switch (message)
    {
    case OPEN :
      
      // cout << "\nSuse_SNS received OPEN" << endl;
      sli = new SuseLocalInfo ;
      sli->Xfinished   = true;
      sli->funfinished = true;
      sli->X.addr = 0;
      sli->Y.addr = 0;  
      sli->fun.addr = 0;
      sli->XVal.addr = 0;
      sli->YVal.addr = 0;
      // get argument configuration info
      qp->Request(args[3].addr, argConfDescriptor);      
      sli->argConfDescriptor = ((CcInt*)argConfDescriptor.addr)->GetIntval();
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
      qp->Request(sli->Y.addr, sli->YVal); // save value of constant argument
      qp->Open(sli->X.addr);               // open the ("outer") input stream
      sli->Xfinished = false;
      sli->fun = SetWord(args[2].addr);
      local = SetWord(sli);
      // cout << "Suse_SNN finished OPEN" << endl;
      return 0;
      
    case REQUEST :
      
      // First, we check whether an inner stream is finished 
      // (sli->funfinished). If so, we try to get a value from 
      // the outer stream and try to re-open the inner stream.
      // sli->X is a pointer to the OUTER stream, 
      // sli->Y is a pointer to the constant argument.

      // cout << "Suse_SNN received REQUEST" << endl;

      // 1. get local data object
      if (local.addr == 0)
	{
	  result.addr = 0;
	  // cout << "Suse_SNN finished REQUEST: CLOSE (1)" << endl;
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
		  // cout << "Suse_SNN finished REQUEST: CLOSE (3)" << endl;
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
	      // cout << "     result.addr=" << result.addr << endl;
	      // cout << "Suse_SNN finished REQUEST: YIELD" << endl;	  
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
      // cout << "Suse_SNN finished REQUEST: CLOSE (4)" << endl;      
      return CANCEL;

    case CLOSE :
      
      // cout << "Suse_SNN received CLOSE" << endl;
      if( local.addr != 0 )
	{
	  sli = (SuseLocalInfo*)local.addr;
	  if (!sli->funfinished)
	    qp->Close( sli->fun.addr ); // close map result stream
	  if (!sli->Xfinished)
	    qp->Close( sli->X.addr );   // close outer stream
	  delete sli;
	}
      // cout << "Suse_SNN finished CLOSE" << endl;
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
  Word              argConfDescriptor;  
  ArgVectorPointer  funargs;

  switch (message)
    {
    case OPEN :
      
      //cout << "\nSuse_SSN received OPEN" << endl;
      sli = new SuseLocalInfo ;
      sli->Xfinished = true;
      sli->Yfinished = true;
      // get argument configuration info
      qp->Request(args[3].addr, argConfDescriptor);      
      sli->argConfDescriptor = ((CcInt*)argConfDescriptor.addr)->GetIntval();
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
      //cout << "Suse_SSN finished OPEN" << endl;
      return 0;
      
    case REQUEST :
      
      // We do a nested loop to join the elements of the outer (sli->X)
      // and inner (sli->Y) stream. For each pairing, we evaluate the
      // parameter function (sli->fun), which return a single result.
      // A clone of the result is passed as the result.
      // We also need to delete each element, when it is not required
      // anymore.

      //cout << "Suse_SSN received REQUEST" << endl;

      // get local data object
      if (local.addr == 0)
	{
	  result.addr = 0;
	  //cout << "Suse_SSN finished REQUEST: CLOSE (1)" << endl;
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
		  //cout << "Suse_SSN finished REQUEST: CANCEL (2)" << endl;
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
	      //cout << "Suse_SSN finished REQUEST: YIELD" << endl;
	      return YIELD;
	    }
	} // end while
      //cout << "Suse_SSN finished REQUEST: CANCEL (3)" << endl;
      return CANCEL;
      
    case CLOSE :
      
      //cout << "Suse_SSN received CLOSE" << endl;
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
      //cout << "Suse_SSN finished CLOSE" << endl;
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
  Word              argConfDescriptor;  
  ArgVectorPointer  funargs;

  switch (message)
    {
    case OPEN :
      
      //cout << "\nSuse_SSS received OPEN" << endl;
      sli = new SuseLocalInfo ;
      sli->Xfinished   = true;
      sli->Yfinished   = true;
      sli->funfinished = true;
      // get argument configuration info
      qp->Request(args[3].addr, argConfDescriptor);      
      sli->argConfDescriptor = ((CcInt*)argConfDescriptor.addr)->GetIntval();
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
      sli->X   = SetWord(args[0].addr); // X is the stream
      sli->Y   = SetWord(args[1].addr); // Y is the constant value
      sli->fun = SetWord(args[2].addr); // fun is the mapping function
      qp->Open(sli->X.addr);            // open X stream argument      
      sli->Xfinished = false;
      local = SetWord(sli);
      //cout << "Suse_SSS finished OPEN" << endl;
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

      //cout << "Suse_SSS received REQUEST" << endl;

      // get local data object
      if (local.addr == 0)
	{
	  result.addr = 0;
	  //cout << "Suse_SSS finished REQUEST: CLOSE (1)" << endl;
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
		  //cout << "Suse_SSS finished REQUEST: CANCEL (2)" << endl;
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
		  //cout << "Suse_SSS finished REQUEST: YIELD" << endl;
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
      //cout << "Suse_SSS finished REQUEST: CANCEL (3)" << endl;
      return CANCEL;
      
    case CLOSE :
      
      //cout << "Suse_SSS received CLOSE" << endl;
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
      //cout << "Suse_SSS finished CLOSE" << endl;
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
  "( ( \"Algebra\" \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>TemporalUnitAlgebra</text--->"
  "<text>For X in kind DATA or X = tuple(Z)*, Y in kind DATA:\n"
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
  "( ( \"Algebra\" \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>TemporalUnitAlgebra</text--->"
  "<text>For X in kind DATA or X = tuple(W)*, Y,Z in kind DATA:\n"
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
types int or point. The distance is always a non-negative ureal value.

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
		     UReal& result, Interval<Instant> iv)
{
  result.timeInterval = iv;
  
  Point rp0, rp1, rp2, rp3;
  double x0, x1, x2, x3, y0, y1, y2, y3, dx1, dy1, dx2, dy2, dt;
  
  p1.TemporalFunction(iv.start, rp0);
  p1.TemporalFunction(iv.end, rp1);
  p2.TemporalFunction(iv.start, rp2);
  p2.TemporalFunction(iv.end, rp3);
  
  dt = iv.end.ToDouble() - iv.start.ToDouble();
  x0 = rp0.GetX(); y0 = rp0.GetY();
  x1 = rp1.GetX(); y1 = rp1.GetY();
  x2 = rp2.GetX(); y2 = rp2.GetY();
  x3 = rp3.GetX(); y3 = rp3.GetY();
  dx1 = (x1 - x0) / dt;
  dy1 = (y1 - y0) / dt;
  dx2 = (x3 - x2) / dt;
  dy2 = (y3 - y2) / dt;

  result.a = pow( (dx1 - dx2), 2 ) + pow( (dy1 - dy2), 2 );
  result.b = 2 * ( (x0 - x2) * (dx1 - dx2) + (y0 - y2) * (dy1 - dy2) );
  result.c = pow( x0 - x2, 2 ) + pow( y0 - y2, 2 );
  result.r = true;
}


int TUDistance_UPoint_UPoint( Word* args, Word& result, int message,
                              Word& local, Supplier s )
{
  Interval<Instant> iv;

  //  Word a1, a2;
  UPoint *u1, *u2;

  if (TUA_DEBUG) cout << "TUDistance_UPoint_UPoint: 0" << endl;
  result = qp->ResultStorage( s );
  if (TUA_DEBUG) cout << "TUDistance_UPoint_UPoint: 1" << endl;

  //  qp->Request(args[0].addr, a1);
  //  qp->Request(args[1].addr, a2);
  //  u1 = (UPoint*)(a1.addr);
  //  u2 = (UPoint*)(a2.addr);

  u1 = (UPoint*)(args[0].addr);
  u2 = (UPoint*)(args[1].addr);

  if (!u1->IsDefined() || 
      !u2->IsDefined() || 
      !u1->timeInterval.Intersects( u2->timeInterval ) )
    { // return undefined ureal
      ((UReal*)(result.addr))->SetDefined( false );
      if (TUA_DEBUG) cout << "TUDistance_UPoint_UPoint: 7" << endl;
    }
  else
    { // get intersection of deftime intervals
      if (TUA_DEBUG) cout << "TUDistance_UPoint_UPoint: 8" << endl;
      u1->timeInterval.Intersection( u2->timeInterval, iv );  
      if (TUA_DEBUG) cout << "TUDistance_UPoint_UPoint: 9" << endl;
      
      // calculate u1, u2, result
      if (TUA_DEBUG) cout << "TUDistance_UPoint_UPoint: 10" << endl;
      UPointDistance( *u1, *u2,  *((UReal*)(result.addr)), iv);
      if (TUA_DEBUG) cout << "TUDistance_UPoint_UPoint: 11" << endl;

    }
  // pass on result
  if (TUA_DEBUG) cout << "TUDistance_UPoint_UPoint: 12" << endl;
  return 0;
}



/*
(a2) value mapping for 

---- (upoint point) -> ureal  and  (point upoint) -> ureal

----

*/

int TUDistance_UPoint_Point( Word* args, Word& result, int message,
                             Word& local, Supplier s )
{
  Word  argConfDescriptor, thePoint, theUPoint;
  int   argConfDescriptor2;

  // get argument configuration
  //  qp->Request(args[2].addr, argConfDescriptor);      
  argConfDescriptor = args[2];
  argConfDescriptor2 = ((CcInt*)argConfDescriptor.addr)->GetIntval();
  if (argConfDescriptor2 == 0) 
    {
      theUPoint = args[0];
      thePoint  = args[1];
      //      qp->Request(args[0].addr, theUPoint);
      //      qp->Request(args[1].addr, thePoint);
    }
  else if (argConfDescriptor2 == 1)
    {
      theUPoint = args[1];
      thePoint  = args[0];
      //      qp->Request(args[1].addr, theUPoint);
      //      qp->Request(args[0].addr, thePoint);
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
      ((UPoint*)(result.addr))->SetDefined ( false );
    }
  else
    ((UPoint*)(theUPoint.addr))->Distance( *((Point*)(thePoint.addr)), 
					   *((UReal*)(result.addr)));
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

  //  qp->Request(args[0].addr, a1);
  //  qp->Request(args[1].addr, a2);
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
      
      c1 = (double) u2->constValue.GetIntval();
      c2 = (double) u2->constValue.GetIntval();
      c = fabs(c1 - c2);
      *((UReal*)(result.addr)) = UReal(iv, 0, 0, c, false);
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
  Word  argConfDescriptor, ii, ui;
  int   argConfDescriptor2;
  UInt  *u;
  CcInt *i;
  double c1, c2, c;

  // get argument configuration
  //qp->Request(args[2].addr, argConfDescriptor);      
  argConfDescriptor = args[2];
  argConfDescriptor2 = ((CcInt*)argConfDescriptor.addr)->GetIntval();
  if (argConfDescriptor2 == 0) 
    {
      //    qp->Request(args[0].addr, ui);
      //    qp->Request(args[1].addr, ii);
      ui = args[0];
      ii = args[1];
    }
  else if (argConfDescriptor2 == 1)
    {
      //    qp->Request(args[1].addr, ii);
      //    qp->Request(args[0].addr, ui);
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
      
      c1 = (double) u->constValue.GetIntval();
      c2 = (double) i->GetIntval();
      c = fabs(c1 - c2);
      *((UReal*)(result.addr)) = UReal(u->timeInterval, 0, 0, c, false);
    }
  // pass on result
  return 0;
}



/*
5.21.3 Specification for operator ~distance~

*/

const string TemporalSpecDistance = 
  "( ( \"Algebra\" \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "(<text>TemporalUnitAlgebra</text--->" 
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
{ TUDistance_UPoint_UPoint,
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
  UReal t_res[3];
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
		   const double& c, const bool r)
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
  double  a, b, c, r;
  int     maxValIndex;
  Instant t = DateTime(instanttype);
  Word    a0;

  result = qp->ResultStorage( s );

  switch (message)
    {
    case OPEN :

      cout << "\nAtExtrURealLocalInfo: OPEN " << endl;
      qp->Request(args[0].addr, a0);
      ureal = (UReal*)(a0.addr);
      cout << "  1" << endl;

      sli = new AtExtrURealLocalInfo;
      sli->NoOfResults = 0;
      sli->ResultsDelivered = 0;
      local = SetWord(sli);
      cout << "  2" << endl;

      if ( !(ureal->IsDefined()) )
	{ // ureal undefined
	  // -> return empty stream
	  cout << "  2.1" << endl;
	  sli->NoOfResults = 0;
	  cout << "AtExtrURealLocalInfo: OPEN  finished (1)" << endl;
	  return 0;
	}
      cout << "  3" << endl;

      if ( (ureal->timeInterval.start).ToDouble() == 
	   (ureal->timeInterval.start).ToDouble() )
	{ // ureal contains only a single point.
	  // -> return a copy of the ureal	  
	  sli->t_res[sli->NoOfResults] = *(ureal->Clone());
	  sli->NoOfResults++;
	  cout << "AtExtrURealLocalInfo: OPEN  finished (2)" << endl;
	  return 0;
	}
      cout << "  4" << endl;

      if (ureal->a == 0)
	{ 
	  if ( ureal->b == 0 )
	    { //  constant function
	      // the only result is a copy of the argument ureal
	      sli->t_res[sli->NoOfResults] = *(ureal->Clone());
	      sli->NoOfResults++;
	      cout << "AtExtrURealLocalInfo: OPEN  finished (3)" << endl;
	      return 0;
	    }
	  if ( ureal->b < 0 )
	    { // linear fuction
	      // the result is a copy of the argument, restricted to
	      // its starting instant
	      sli->t_res[sli->NoOfResults] = *(ureal->Clone());
	      sli->t_res[sli->NoOfResults].timeInterval.end =
		sli->t_res[sli->NoOfResults].timeInterval.start;
	      sli->t_res[sli->NoOfResults].timeInterval.rc = true;
	      sli->NoOfResults++;
	      cout << "AtExtrURealLocalInfo: OPEN  finished (4)" << endl;
	      return 0;
	    }
	  if ( ureal->b > 0 )
	    { // linear fuction
	      // the result is a copy of the argument, restricted to
	      // its final instant
	      sli->t_res[sli->NoOfResults] = *(ureal->Clone());
	      sli->t_res[sli->NoOfResults].timeInterval.start =
		sli->t_res[sli->NoOfResults].timeInterval.end;
	      sli->t_res[sli->NoOfResults].timeInterval.lc = true;	      
	      sli->NoOfResults++;
	      cout << "AtExtrURealLocalInfo: OPEN  finished (5)" << endl;
	      return 0;
	    }
	}
      cout << "  5" << endl;

      if (ureal->a !=0) 
	{ // quadratic function
	  // we have to additionally check for the extremum 
	  cout << "  5.1" << endl;
	  
	  // get the times of interest
	  a = ureal->a;
	  b = ureal->b;
	  c = ureal->c;
	  r = ureal->r;
	  t_extr  = -b/a; 
	  t_start =   0.0;
	  t_end   =   (ureal->timeInterval.end).ToDouble() 
	            - (ureal->timeInterval.start).ToDouble();
	  // get the values of interest
	  v_extr  = getValUreal(t_extr, a,b,c,r);
	  v_start = getValUreal(t_start,a,b,c,r);
	  v_end   = getValUreal(t_end,  a,b,c,r);
	  cout << "  5.2" << endl;

	  // compute, which values are maximal
	  if ( (t_start <= t_extr) && (t_end   >= t_extr) )
	    {
	      cout << "  5.3" << endl;
	      maxValIndex = getMaxValIndex(v_extr,v_start,v_end);
	    }
	  else 
	    { 
	      cout << "  5.4" << endl;
	      maxValIndex = 0;
	      if (v_start >= v_end) 
		maxValIndex += 2;
	      if (v_end >= v_start) 
		maxValIndex += 4;
	    }
	  cout << "  5.5" << endl;
	  if (maxValIndex & 2)
	    { // start value
	      cout << "  5.6" << endl;
	      sli->t_res[sli->NoOfResults] = *(ureal->Clone());
	      t.ReadFrom(t_start + (ureal->timeInterval.start).ToDouble());
	      Interval<Instant> i( t, t, true, true );
	      sli->t_res[sli->NoOfResults].timeInterval = i;
	      sli->NoOfResults++;
	    }
	  if ( ( maxValIndex & 4 ) && ( t_end != t_start ) )
	    { // end value
	      cout << "  5.7" << endl;
	      sli->t_res[sli->NoOfResults] = *(ureal->Clone());
	      t.ReadFrom(t_end + (ureal->timeInterval.start).ToDouble());
	      Interval<Instant> i( t, t, true, true );
	      sli->t_res[sli->NoOfResults].timeInterval = i;
	      sli->NoOfResults++;
	    }
	  
	  if ( (maxValIndex & 1)   && 
               (t_extr != t_start) && 
               (t_extr != t_end)      )	    
	    {
	      cout << "  5.8" << endl;
	      sli->t_res[sli->NoOfResults] = *(ureal->Clone());
	      t.ReadFrom(t_extr + (ureal->timeInterval.start).ToDouble());
	      Interval<Instant> i( t, t, true, true );
	      sli->t_res[sli->NoOfResults].timeInterval = i;
	      sli->NoOfResults++;
	    }
	  cout << "  5.9" << endl;
	  return 0;
	}
      cout << "  6" << endl;
      cout << "\natmaxUReal (OPEN): This should not happen!" << endl;
      cout << "AtExtrURealLocalInfo: OPEN  finished (6)" << endl;
      return 0;

    case REQUEST :

      cout << "\nAtExtrURealLocalInfo: REQUEST" << endl;
      if (local.addr == 0)
	{
	  cout << "AtExtrURealLocalInfo: REQUEST CANCEL(1)" << endl;
	  return CANCEL;
	}
      sli = (AtExtrURealLocalInfo*) local.addr;
      
      if (sli->NoOfResults <= sli->ResultsDelivered)
	{
	  cout << "AtExtrURealLocalInfo: REQUEST CANCEL (2)" << endl;
	  return CANCEL;
	}
      result = SetWord( sli->t_res[sli->ResultsDelivered].Clone() );
      sli->t_res[sli->ResultsDelivered].DeleteIfAllowed();
      sli->ResultsDelivered++;
      cout << "AtExtrURealLocalInfo: REQUEST YIELD" << endl;
      return YIELD;
      
    case CLOSE :

      cout << "\nAtExtrURealLocalInfo: CLOSE" << endl;
      if (local.addr != 0)
	{
	  sli = (AtExtrURealLocalInfo*) local.addr;
	  while (sli->NoOfResults > sli->ResultsDelivered)
	    {
	      sli->t_res[sli->ResultsDelivered].DeleteIfAllowed();
	      sli->ResultsDelivered++;
	    }
	  delete sli;
	}
      cout << "AtExtrURealLocalInfo: CLOSE finished" << endl;
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
  "( ( \"Algebra\" \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "(<text>TemporalUnitAlgebra</text--->" 
  "<text>For T in {int, bool, string}:\n"
  "uT    -> uT\n"
  "ureal -> (stream ureal)</text--->"
  "<text> atmax( _ )</text--->"
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
  double  a, b, c, r;
  int     minValIndex;
  Instant t;
  Word    a0;

  result = qp->ResultStorage( s );

  switch (message)
    {
    case OPEN :

      qp->Request(args[0].addr, a0);
      ureal = (UReal*)(a0.addr);

      sli = new AtExtrURealLocalInfo;
      sli->NoOfResults = 0;
      sli->ResultsDelivered = 0;
      local = SetWord(sli);

      if ( !ureal->IsDefined() )
	{ // ureal undefined
	  // -> return empty stream
	  sli->NoOfResults = 0;
	  return 0;
	}

      if ( (ureal->timeInterval.start).ToDouble() == 
	   (ureal->timeInterval.start).ToDouble() )
	{ // ureal contains only a single point.
	  // -> return a copy of the ureal	  
	  sli->t_res[sli->NoOfResults] = *(ureal->Clone());
	  sli->NoOfResults++;
	  return 0;
	}

      if (ureal->a == 0)
	{ 
	  if ( ureal->b == 0 )
	    { //  constant function
	      // the only result is a copy of the argument ureal
	      sli->t_res[sli->NoOfResults] = *(ureal->Clone());
	      sli->NoOfResults++;
	      return 0;
	    }
	  if ( ureal->b < 0 )
	    { // linear fuction
	      // the result is a copy of the argument, restricted to
	      // its ending instant
	      sli->t_res[sli->NoOfResults] = *(ureal->Clone());
	      sli->t_res[sli->NoOfResults].timeInterval.end =
		sli->t_res[sli->NoOfResults].timeInterval.start;
	      sli->t_res[sli->NoOfResults].timeInterval.lc = true;
	      sli->NoOfResults++;
	      return 0;
	    }
	  if ( ureal->b > 0 )
	    { // linear fuction
	      // the result is a copy of the argument, restricted to
	      // its starting instant
	      sli->t_res[sli->NoOfResults] = *(ureal->Clone());
	      sli->t_res[sli->NoOfResults].timeInterval.start =
		sli->t_res[sli->NoOfResults].timeInterval.end;
	      sli->t_res[sli->NoOfResults].timeInterval.rc = true;
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
	  t_extr  = -b/a; 
	  t_start = 0.0;
	  t_end   =   (ureal->timeInterval.end).ToDouble()
	            - (ureal->timeInterval.start).ToDouble();
	  // get the values of interest
	  v_extr  = getValUreal(t_extr, a,b,c,r);
	  v_start = getValUreal(t_start,a,b,c,r);
	  v_end   = getValUreal(t_end,  a,b,c,r);
	  // compute, which values are minimal

	  if ( (t_start <= t_extr) && (t_end   >= t_extr) )
	    minValIndex = getMinValIndex(v_extr,v_start,v_end);
	  else 
	    { 
	      minValIndex = 0;
	      if (v_start <= v_end) 
		minValIndex += 2;
	      if (v_end <= v_start) 
		minValIndex += 4;
	    }

	  if (minValIndex & 2)
	    {
	      sli->t_res[sli->NoOfResults] = *(ureal->Clone());
	      t.ReadFrom(t_start + (ureal->timeInterval.start).ToDouble());
	      Interval<Instant> i( t, t, true, true );
	      sli->t_res[sli->NoOfResults].timeInterval = i;
	      sli->NoOfResults++;
	    }
	  if ( (minValIndex & 4) && (t_start != t_end) )
	    {
	      sli->t_res[sli->NoOfResults] = *(ureal->Clone());
	      t.ReadFrom(t_end + (ureal->timeInterval.start).ToDouble());
	      Interval<Instant> i( t, t, true, true );
	      sli->t_res[sli->NoOfResults].timeInterval = i;
	      sli->NoOfResults++;
	    }
	  if ( (minValIndex & 1)   &&
	       (t_extr != t_start) &&
	       (t_extr != t_end)      )
	    {
	      sli->t_res[sli->NoOfResults] = *(ureal->Clone());
	      t.ReadFrom(t_extr + (ureal->timeInterval.start).ToDouble());
	      Interval<Instant> i( t, t, true, true );
	      sli->t_res[sli->NoOfResults].timeInterval = i;
	      sli->NoOfResults++;
	    }
	  sli->NoOfResults++;
	  return 0;
	}
      cout << "\natminUReal (OPEN): This should not happen!" << endl;
      sli->NoOfResults++;
      return 0;

    case REQUEST :

      if (local.addr == 0)
	return CANCEL;
      sli = (AtExtrURealLocalInfo*) local.addr;
      
      if (sli->NoOfResults <= sli->ResultsDelivered)
	return CANCEL;

      result = SetWord( sli->t_res[sli->ResultsDelivered].Clone() );
      sli->t_res[sli->ResultsDelivered].DeleteIfAllowed();
      sli->ResultsDelivered++;
      return YIELD;
      
    case CLOSE :

      if (local.addr != 0)
	{
	  sli = (AtExtrURealLocalInfo*) local.addr;
	  while (sli->NoOfResults > sli->ResultsDelivered)
	    {
	      sli->t_res[sli->ResultsDelivered].DeleteIfAllowed();
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
  "( ( \"Algebra\" \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "(<text>TemporalUnitAlgebra</text--->" 
  "<text>For T in {int, bool, string}:\n"
  "uT    -> uT\n"
  "ureal -> (stream ureal)</text--->"
  "<text> atmin( _ )</text--->"
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
       saggregate: (stream T) x (T x T --> T) x T --> T

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
      ErrorReporter::ReportError("Operator saggregate expects a list of length"
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
      ErrorReporter::ReportError("Operator saggregate expects a list of length"
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
  "( ( \"Algebra\" \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "(<text>TemporalUnitAlgebra</text--->" 
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
  "fun(i1:int, i2:int) ifthenelse(i1>i2,i1,i2) ; 0]</text--->"
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

    intersection: uT x uT --> (stream uT)         (all but T in {point, real})
                  uT x  T --> (stream uT)               Test
                   T x uT --> (stream uT)               Test
                  upoint x line --> (stream upoint)      --
		  line x upoint --> (stream upoint)      --
                  upoint x uregion (stream upoint)       --

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
    if( nl->IsEqual( arg1, "upoint" ) && nl->IsEqual( arg2, "uregion") )
      return  nl->TwoElemList(nl->SymbolAtom( "stream" ),
			      nl->SymbolAtom( "upoint" ));  
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
    Word resultValues[2];
    int  NoOfResults;
    int  NoOfResultsDelivered;
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
	  // store result
	  sli->resultValues[sli->NoOfResults] = SetWord( uv1->Clone() );
	  ((T*)(result.addr))->timeInterval = iv;
	  sli->NoOfResults++;	  
	}// else: no result
      local = SetWord(sli);
      return 0;

    case REQUEST:
      
      if(local.addr == 0)
	return CANCEL;
      sli = (TUIntersectionLocalInfo*) local.addr;
      if(sli->finished)
	return CANCEL;
      if(sli->NoOfResultsDelivered < sli->NoOfResults)
	{
	  result = SetWord( ((T*)
	     (sli->resultValues[sli->NoOfResultsDelivered].addr))->Clone() );   
	  ((T*)(sli->resultValues[sli->NoOfResultsDelivered].addr))
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

      uv1 = (UT*) (u1.addr);
      uv2 = (T*) (u2.addr);
	

      if ( uv1->IsDefined() && 
	   uv2->IsDefined() && 
	   (uv1->constValue.Compare( uv2 ) == 0 ) )	   
	{ // store result
	  sli->resultValues[sli->NoOfResults] = SetWord( uv1->Clone() );
	  sli->NoOfResults++;	  
	}// else: no result
      local = SetWord(sli);
      return 0;

    case REQUEST:
      
      if(local.addr == 0)
	return CANCEL;
      sli = (TUIntersectionLocalInfo*) local.addr;
      if(sli->finished)
	return CANCEL;
      if(sli->NoOfResultsDelivered < sli->NoOfResults)
	{
	  result = SetWord( ((UT*)
	     (sli->resultValues[sli->NoOfResultsDelivered].addr))->Clone() );   
	  ((UT*)(sli->resultValues[sli->NoOfResultsDelivered].addr))
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
  double radicand, a, b, c, r, y;
  DateTime t1, t2;
  Interval<Instant> rdeftime, deftime;
  Word a0, a1;
  UReal *uinput;
  

  switch (message)
    {
    case OPEN :
      
      localinfo = new MappingUnitAt_rLocalInfo;
      localinfo->finished = true;
      localinfo->NoOfResults = 0;
      
      //      qp->Request(args[0].addr, a0);
      //      qp->Request(args[1].addr, a1);

      // initialize arguments, such that a0 always contains the ureal
      //                       and a1 the real 
      if (uargindex == 0)
	{ a0 = args[0]; a1 = args[1]; }
      else
	{ a0 = args[1]; a1 = args[0]; }

      uinput = ((UReal*)(a0.addr));
      y = ((CcReal*)(a1.addr))->GetRealval();

      if ( !uinput->IsDefined() ||
	   !((CcReal*)(a1.addr))->IsDefined() )
	{ // some input is undefined -> return empty stream
	  localinfo->NoOfResults = 0;
	  localinfo->finished = true;
	  local = SetWord(localinfo);
	  return 0;
	}
      
      a = uinput->a;
      b = uinput->b;
      c = uinput->c;
      r = uinput->r;
      deftime = uinput->timeInterval;

      if ( (a == 0) && (b == 0) )
	{ // constant function. Possibly return input unit
	  if (c != y)
	    { // There will be no result, just an empty stream
	      localinfo->NoOfResults = 0;
	      localinfo->finished = true;
	    }
	  else
		{ // Return the complete unit
		  (UReal*)(localinfo->runits[localinfo->NoOfResults].addr)
		    = uinput->Copy();
		  localinfo->NoOfResults++;
		  localinfo->finished = false;
		}
	  local = SetWord(localinfo);
	  return 0;
	}
      if ( (a == 0) && (b != 0) )
	{ // linear function. Possibly return input unit restricted 
	  // to single value
	  t1.ReadFrom( (y - c)/b );
	  if (deftime.Contains(t1))
	    { // value is contained by deftime
	      (UReal*)(localinfo->runits[localinfo->NoOfResults].addr) = 
		uinput->Copy();
	      ((UReal*)(localinfo
			->runits[localinfo->NoOfResults].addr))
		->timeInterval = Interval<Instant>(t1, t1, true, true);
	      localinfo->NoOfResults++;
	      localinfo->finished = false;		  
	    }
	  else
	    { // value is not contained by deftime -> no result
	      localinfo->NoOfResults = 0;
	      localinfo->finished = true;
	    }
	  local = SetWord(localinfo);
	  return 0;
	}
      
      radicand = ((y - c) / a) + ((b * b) / (4 * a * a));
      if ( (a != 0) && (radicand <= 0) )
	{ // quadratic function. There are possibly two result units
	  // calculate the possible t-values t1, t2
	  
	  t1.ReadFrom( sqrt(radicand) );
	  t2.ReadFrom( -sqrt(radicand) );
	  
	  // check, whether t1 contained by deftime
	  if (deftime.Contains(Instant(t1)))
	    {
	      rdeftime.start = t1;
	      rdeftime.end = t1;
	      localinfo->runits[localinfo->NoOfResults].addr = 
		new UReal( rdeftime,a,b,c,r );
	      localinfo->NoOfResults++;
	      localinfo->finished = false;
	    }
	  // check, whether t2 contained by deftime
	  if (deftime.Contains( t2 ))
	    {
	      rdeftime.start = t2;
	      rdeftime.end = t2;
	      localinfo->runits[localinfo->NoOfResults].addr = 
		new UReal( rdeftime,a,b,c,r );
	      localinfo->NoOfResults++;
	      localinfo->finished = false;
	    }
	}
      else // there is no result unit
	{
	  localinfo->NoOfResults = 0;
	  localinfo->finished = true;
	}
      local = SetWord(localinfo);
      return 0;
      
    case REQUEST :
      
      if (local.addr == 0)
	return CANCEL;
      localinfo = (MappingUnitAt_rLocalInfo*) local.addr;
      if (localinfo->finished)
	return CANCEL;
      if ( localinfo->NoOfResults <= 0 )
	{ localinfo->finished = true;
	  return CANCEL;
	}
      localinfo->NoOfResults--;
      result = SetWord( ((UReal*)(localinfo
				  ->runits[localinfo->NoOfResults].addr))
			->Clone() );
      ((UReal*)(localinfo->runits[localinfo->NoOfResults].addr))
	->DeleteIfAllowed();
      return YIELD;
      
    case CLOSE :

      if (local.addr != 0)
	{
	  localinfo = (MappingUnitAt_rLocalInfo*) local.addr;
	  for (;localinfo->NoOfResults>0;localinfo->NoOfResults--)
	    ((UReal*)(localinfo->runits[localinfo->NoOfResults].addr))
	      ->DeleteIfAllowed();
	  delete localinfo;
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

Test, whether both upoints have identical direction vectors. If so,
return one argument restricted to the intersection of timeintervals.

Otherwise there will be at most one possible point of intersection.


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
  Point p1n_start, p1n_end, p2n_start, p2n_end, p_intersect;
  UPoint p1norm, p2norm;
  double t_x, t_y, t1, t2, dxp1, dxp2, dyp1, dyp2;
  Coord px11, px12, px21, px22, py11, py12, py21, py22;
  double p1max[2], p1min[2], p2max[2], p2min[2];
  Rectangle<2> mbb1, mbb2;
  
  // test for overlapping intervals
  switch( message )
    {
    case OPEN:
      
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
	return 0; // nothing to do

      // get common time interval
      uv1->timeInterval.Intersection(uv2->timeInterval, iv);

      // normalize both starting and ending points to interval
      uv1->TemporalFunction( iv.start, p1n_start);
      uv1->TemporalFunction( iv.end, p1n_end);
      p1norm = UPoint( iv, p1n_start, p1n_end );

      uv2->TemporalFunction( iv.start, p2n_start);
      uv2->TemporalFunction( iv.end, p2n_end);
      p2norm = UPoint( iv, p2n_start, p2n_end );

      // test MBBs:
      px11 = p1n_start.GetX(); py11 = p1n_start.GetY();
      px12 = p1n_end.GetX();   py12 = p1n_end.GetY();
      px21 = p2n_start.GetX(); py21 = p2n_start.GetY();
      px22 = p2n_end.GetX();   py22 = p2n_end.GetY();

      p1min[0] = min(px11,px12); p1max[0] = max(px11,px12);
      p1min[1] = min(py11,py12); p1max[1] = max(py11,py12);
      p2min[0] = min(px21,px22); p2max[0] = max(px21,px22);
      p2min[1] = min(py21,py22); p2max[1] = max(py21,py22);
      
      mbb1 = Rectangle<2>(true, p1min, p1max);
      mbb2 = Rectangle<2>(true, p2min, p2max);
      
      if (!mbb1.Intersects(mbb2))
	return 0; // no intersection

      // test for identity:
      if ( p1norm.EqualValue( p2norm ))
	{ // both upoints have the same linear function
	  sli->resultValues[sli->NoOfResults] = SetWord( p1norm.Clone() );
	  sli->NoOfResults++;	  	  
	  return 0;
	}

      // test for parallelity: they are parallel,
      // if dx1 = dx2 and dy1 = dy2 and they don't have a common 
      // starting or ending point
      dxp1 = ((double) px12) - ((double) px11);
      dyp1 = ((double) py12) - ((double) py11);
      dxp2 = ((double) px22) - ((double) px21);
      dyp2 = ((double) py22) - ((double) py21);
      
      if ( AlmostEqual(dxp1, dxp2) && AlmostEqual(dyp1, dyp2)      &&
	   ( !AlmostEqual(px11,px21) || !AlmostEqual(py11,py21) )  &&
	   ( !AlmostEqual(px12,px22) || !AlmostEqual(py12,py22) )   )
	return 0; // they are parallel -> no intersection

/*     
Trying to find an intersection point $t$ with $A_1t + B_1 = A_2t + B_2$ 
we get:
    
\[ t_x = \frac{px_{21} - px_{11}}{dxp_1 - dxp_2} \quad
t_y = \frac{py_{21} - py_{11}}{dyp_1 - pyp_2} \]
     
where $t = t_x = t_y$. If $t_x \neq t_y$, then there is no intersection!

*/

      if ( ( (dxp1-dxp2) == 0 ) || ( (dyp1-dyp2) == 0 ) )
	{
	  cout << "\nWARNING: in temporalUnitIntersection_upoint_upoint:" 
	       << "Trajectories seem to be parallel, though that has not been"
	       << "been detected before." << endl;
	  return 0;
	}

      t_x = (px21-px11) / (dxp1-dxp2);
      t_y = (py21-py11) / (dyp1-dyp2);      

      t1 = iv.start.ToDouble();
      t2 = iv.end.ToDouble();
	
      if ( AlmostEqual(t_x, t_y) &&
	   ( t_x >= t1) &&
	   ( t_x <= t2) )
	{ // We found an intersection
	  t.ReadFrom(t1); // create Instant
	  iv = Interval<Instant>( t, t, true, true ); // create Interval
	  sli->resultValues[sli->NoOfResults] = SetWord( p1norm.Clone() );
	  ((UPoint*)(sli->resultValues[sli->NoOfResults].addr))
            ->timeInterval=iv;
	  sli->NoOfResults++;	  	  	  
	}
      
      // else: no result
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

// value mapping for (upoint point) -> (stream upoint)
//               and (upoint point) -> (stream upoint)
// is identical with at: (upoint point) -> (stream upoint).
// We just add switches for both signatures and the stream framework
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

// for (upoint line) -> (stream upoint)
template<int uargindex>
int
temporalUnitIntersection_upoint_line( Word* args, Word& result, int message,
		                      Word& local, Supplier s )
{
  cout << "\nATTENTION: temporalUnitIntersection_upoint_line "
       << "not yet implemented!" << endl;  
  return 0;
}

// for (upoint uregion) -> (stream upoint)
int
temporalUnitIntersection_upoint_uregion( Word* args, Word& result, int message,
		                         Word& local, Supplier s )
{
  TUIntersectionLocalInfo *sli;
  Word a0, a1;
  UPoint  *upoint, pResult;
  URegion *uregion;
  Point *val;
  Interval<Instant> iv;

  cout << "\nATTENTION: temporalUnitIntersection_upoint_uregion "
       << "not yet implemented!" << endl;  
  return 0;

  ////////////////////////////////////////////////////////////////////////
  // Only a framework...

  switch( message )
    {
    case OPEN:
      
      sli = new TUIntersectionLocalInfo;
      sli->finished = true;
      sli->NoOfResults = 0;
      sli->NoOfResultsDelivered = 0;
      local = SetWord(sli);

      upoint = (UPoint*)(args[0].addr);
      uregion = (URegion*)(args[1].addr);

      // test for definedness and intersection of deftimes
      if ( !upoint->IsDefined() ||
	   !uregion->IsDefined() ||
	   !upoint->timeInterval.Intersects( uregion->timeInterval ) )
	return 0; // nothing to do

      // get common time interval
      upoint->timeInterval.Intersection(uregion->timeInterval, iv);
      
      //////////////////////////////////////////////
      // extend this section to implement creation 
      // of result stream elements:

      if ( false )
	{
	  pResult.SetDefined(true);
	  pResult.timeInterval.start.SetDefined(true);
	  pResult.timeInterval.end.SetDefined(true);
	  sli->resultValues[sli->NoOfResults] = SetWord( pResult.Clone() );
	  sli->NoOfResults++;	  
	}

      return 0;

      //////////////////////////////////////////////
      // Nothing do do from here on:

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
5.24.3 Specification for operator ~intersection~

*/

const string  TemporalUnitIntersectionSpec  =
  "( ( \"Algebra\" \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "(<text>TemporalUnitAlgebra</text--->" 
  "<text>For T in {bool, int, real, string, point}:\n"
  "(uT uT) -> (stream uT)**\n"
  "(uT  T) -> (stream uT)\n"
  "( T uT) -> (stream uT)\n"
  "(line upoint) -> (stream upoint)*\n"
  "(upoint line) -> (stream upoint)*\n"
  "(upoint uregion) -> (stream upoint)*\n"
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

    temporalUnitIntersection_upoint_uregion
  };

int temporalunitIntersectionSelect( ListExpr args )
{
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  if( nl->IsEqual( arg1, "ubool" ) )   return 0;
  if( nl->IsEqual( arg1, "uint" ) )    return 1;
  if( nl->IsEqual( arg1, "ureal" ) )   return 2;
  if( nl->IsEqual( arg1, "upoint" ) )  return 3;
  if( nl->IsEqual( arg1, "ustring" ) ) return 4;

  if( nl->IsEqual( arg1, "ubool" ) )   return 5;
  if( nl->IsEqual( arg1, "uint" ) )    return 6;
  if( nl->IsEqual( arg1, "ureal" ) )   return 7;
  if( nl->IsEqual( arg1, "upoint" ) &&
      nl->IsEqual( arg2, "point"  ) )  return 8;
  if( nl->IsEqual( arg1, "ustring" ) ) return 9;

  if( nl->IsEqual( arg1, "ubool" ) )   return 10;
  if( nl->IsEqual( arg1, "uint" ) )    return 11;
  if( nl->IsEqual( arg1, "ureal" ) )   return 12;
  if( nl->IsEqual( arg1, "upoint" ) )  return 13;
  if( nl->IsEqual( arg1, "ustring" ) ) return 14;

  if( nl->IsEqual( arg1, "line" ) )    return 15;
  if( nl->IsEqual( arg1, "upoint" ) &&
      nl->IsEqual( arg2, "line" ) )    return 16;
  if( nl->IsEqual( arg1, "upoint" ) &&
      nl->IsEqual( arg2, "uregion" ) ) return 17;
  cout << "\ntemporalunitIntersectionSelect: Unsupported datatype." << endl;
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
5.26 Operator ~transformstream~

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
      (Attribute*)(result.addr) = tupleptr->GetAttribute(0)->Clone();
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
  "( ( \"Algebra\" \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "(<text>TemporalUnitAlgebra</text--->" 
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
6 Creating the Algebra

*/

class TemporalUnitAlgebra : public Algebra
{
 public:
  TemporalUnitAlgebra() : Algebra()
  {
   AddOperator( &temporalunitmakemvalue );
   AddOperator( &temporalunittransformstream );
   AddOperator( &temporalunitsfeed );
   AddOperator( &temporalunitsuse );
   AddOperator( &temporalunitsuse2 );
   AddOperator( &temporalunitsaggregate );
   AddOperator( &temporalunitqueryrect2d );
   AddOperator( &temporalunitpoint2d );
   AddOperator( &temporalcircle );
   AddOperator( &temporalmakepoint );
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


